#!/usr/bin/perl
# Parses the network package definitions
# uses the results to make encoder & decoder functions for DBProcess and
# to make the add-to-package code in xxxObject.cpp

use Getopt::Long;

&main;

# ======================================================================

sub main
{
    &GetOptions("checkout","source=s","encoder=s","decoder=s","datafile=s","loadobject=s","windows");
    
    if ($opt_datafile eq "")
    {
	print "Usage:\n";
#    print "--checkout   Check out all files that might be changed"
	print "--datafile <filename>    Specify the file to read package data from.\n";
	print "--source <filename>      Write package add functions to file (for the serverGame library).\n";
	print "--encoder <filename>     Write Encoder functions for SwgDatabaseServer.\n";
	print "--decoder <filename>     Write Decoder functions for SwgDatabaseServer.\n";
	print "--loadobject <filename>  Write LoadObject functions for SwgDatabaseServer.\n";
	print "--windows                Use this option when running under Windows.\n";
	die;
    }
    
    open (DATAFILE,$opt_datafile);
    while (<DATAFILE>)
    {
	chop;
	s/\#.*//;
	next if (/^\s*$/);
	last if (/^\s*end\s*$/);
	
	s/\s+/\t/g;
	($classname,$parentname)=split("\t",$_);
#    print "class \"${classname}\" parent \"${parentname}\"\n";
	if ($classname =~ /^\-/)
	{
	    $classname =~ s/^-//;
	}
	else
	{
	    $parent{$classname}=$parentname;
	}
	push(@packableClasses,$classname);
    }
    
    while (<DATAFILE>)
    {
	chop;
	s/\#.*//;
	next if (/^\s*$/);
	
	s/\s+/\t/g;
	($class,$cName,$package,$datatype,$addCommand,$persistFunction,$loadFunction)=split("\t",$_); #split("\s+" ????
	$package="client" if ($package eq "authClientServer");
	$package="parentClient" if ($package eq "firstParentAuthClientServer");
	$addCommand="" if ($addCommand eq "-");
	$memberdata = [$cName, $datatype, $addCommand, $persistFunction, $loadFunction];
	push @{ $packageMembers{"$class.$package"} },$memberdata;
    }
    close (DATAFILE);
	
    foreach $package (keys(%packageMembers))
    {
#    print "$package count is ".scalar(@{ $packageMembers{$package} })."\n";
	foreach $member (@{ $packageMembers{$package} })
	{
	    ($a,$b,$c)=@{ $member };
#	print "$package\t$a\t$b\t$c\n";
	}
    }
    
#print out packaging functions
    
    if ($opt_source ne "")
    {
	$filename = $opt_source;
	&editFile($filename);
    }
    
    if ($opt_encoder ne "")
    {
	$filename = $opt_encoder;
	&editEncoder($filename);
    }
    
    if ($opt_decoder ne "")
    {
	$filename = $opt_decoder;
	&editDecoder($filename);
    }

    if ($opt_loadobject ne "")
    {
	$filename = $opt_loadobject;
	&editLoadObject($filename);
    }
}

# ----------------------------------------------------------------------

sub editFile
{
    my($filename)=@_;
    my($classname);

#    $classname = $filename;
#    $classname =~ s/\.cpp//;
#    $classname =~ s/.*\/([^\/]+)$/$1/;

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED PACKAGEADD/)
	{
	    foreach $classname (sort (@packableClasses))
	    {
		print OUTFILE "/*\n";
		print OUTFILE " * Generated function.  Do not edit.\n";
		print OUTFILE " */\n";
		print OUTFILE "void ${classname}::addMembersToPackages()\n";
		print OUTFILE "{\n";

		&printPackage($classname,"server");
		&printPackage($classname,"shared");
		&printPackage($classname,"client"); 
		&printPackage($classname,"server_np");
		&printPackage($classname,"shared_np");
		&printPackage($classname,"client_np");
		&printPackage($classname,"authClientServer_np");
		&printPackage($classname,"parentClient");
		&printPackage($classname,"firstParentAuthClientServer_np");

		print OUTFILE "}\n";
		print OUTFILE "\n";
	    }

	    while (!/!!!END GENERATED PACKAGEADD/)
	    {
		#eat old packageadd commands
		die unless ($_=<INFILE>);
	    }
	    print OUTFILE $_;
	}
    }
    close (INFILE);
    close (OUTFILE);
    if ($opt_windows == 1)
    {
	system ("copy ${filename}.make_encoder_temporary_file $filename /y");
	system ("del ${filename}.make_encoder_temporary_file");
    }
    else
    {
	system ("cp ${filename}.make_encoder_temporary_file $filename");
	system ("rm ${filename}.make_encoder_temporary_file");
    }
}

# ----------------------------------------------------------------------

sub printPackage
{
    my($class,$package)=@_;
    my($member, $cName, $datatype, $addCommand, $persistFunction);

    foreach $member (@{ $packageMembers{"$class.$package"} })
    {
	($cName, $datatype, $addCommand, $persistFunction)=@{ $member };
	
	if ($addCommand eq "")
	{
	    $realpackagename = $package;
	    $realpackagename =~ s/parentClient$/firstParentAuthClientServer/;
	    $realpackagename =~ s/client$/authClientServer/;
	    if ($realpackagename =~ /\_np/)
	    {
		$realpackagename =~ s/\_np//;
		$realpackagename.="Variable_np";
	    }
	    else
	    {
		$realpackagename.="Variable   ";
	    }

	    print OUTFILE "\tadd\u${realpackagename} ($cName);\n";
	}
	else
	{
	    print OUTFILE "\t$addCommand\n";
	}
    }   
}

# ----------------------------------------------------------------------

sub editEncoder
{
    my($filename)=@_;
    my($classname);

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED ENCODER/)
	{
	    foreach $classname (sort keys (%parent))
	    {
#	    foreach $classname ("VehicleObject",
#				"TangibleObject" ,
#				"UniverseObject",
#				"ResourceClassObject",
#				"ResourceTypeObject",
#				"CellObject",
#				"InstallationObject",
#				"CreatureObject",
#				"StaticObject",
#				"ServerObject",
#				"ResourcePoolObject",
#				"BuildingObject",
#				"WeaponObject",
#				"PlanetObject")
#	    {
		&makeEncodeFunction($classname,"shared");
		&makeEncodeFunction($classname,"server");
		&makeEncodeFunction($classname,"client");
	    }
	    &makeEncodeFunction("PlayerObject","parentClient");
	
	    &makeEncodeSwitcher("shared");
	    &makeEncodeSwitcher("server");
	    &makeEncodeSwitcher("client");
	    &makeEncodeSwitcher("parentClient");

	    while (!/!!!END GENERATED ENCODER/)
	    {
		#eat old generated code
		die unless ($_=<INFILE>);
	    }
	    print OUTFILE $_;
	}
    } 

    close (INFILE);
    close (OUTFILE);

    if ($opt_windows == 1)
    {
	system ("copy ${filename}.make_encoder_temporary_file $filename");
	system ("del ${filename}.make_encoder_temporary_file");
    }
    else
    {
	system ("cp ${filename}.make_encoder_temporary_file $filename");
	system ("rm ${filename}.make_encoder_temporary_file");
    }
}

# ----------------------------------------------------------------------

sub makeEncodeFunction
{
    my ($classname,$package)=@_;
    my ($member,$rowtype,$buffer);

    print OUTFILE "bool SwgSnapshot::encode\u${package}${classname}(NetworkId const & objectId, Archive::ByteStream &data, bool addCount) const\n";
    print OUTFILE "{\n";
    print OUTFILE "\tif (addCount)\n";
    print OUTFILE "\t{\n";
    print OUTFILE "\t\tuint16 temp=".&packageCount($classname,$package).";\n";
    print OUTFILE "\t\tArchive::put(data,temp);\n";
    print OUTFILE "\t}\n";
    if (($parent{$classname} ne "") && (&packageCount($parent{$classname},$package)!=0))
    {
	# a parent that cannot be encoded fails the child: see SwgSnapshot::prepareSend
	print OUTFILE "\tif (!encode\u${package}".$parent{$classname}."(objectId, data, false))\n";
	print OUTFILE "\t\treturn false;\n";
	print OUTFILE "\n";
    }
    print OUTFILE "\tPROFILER_AUTO_BLOCK_DEFINE(\"encode\u${package}${classname}\");\n";
    print OUTFILE "\n";
    if ($classname eq "ServerObject")
    {
	$rowtype="ObjectBufferRow";
	$buffer="objectTableBuffer";
    }
    else
    {
	if (!($classname =~ /Object$/))
	{
	    $rowtype=$classname."ObjectBufferRow";
	    $buffer=$classname."ObjectBuffer";
	}
	else
	{
	    $rowtype=$classname."BufferRow";
	    $buffer=$classname."Buffer";
	}
    }
    print OUTFILE "\tconst DBSchema::${rowtype} *row=m_\l${buffer}.findConstRowByIndex(objectId);\n";
    print OUTFILE "\tWARNING_STRICT_FATAL(row==NULL,(\"Loading object %s, no ${rowtype} in the buffer\\n\",objectId.getValueString().c_str()));\n";
    print OUTFILE "\tif (!row) {\n";
    print OUTFILE "\t\treturn false;\n";
    print OUTFILE "\t}\n";
    print OUTFILE "\n";

    foreach $member (@{ $packageMembers{"$classname.$package"} })
    {
	($cName, $datatype, $addCommand, $persistFunction)=@{ $member };

	if ($persistFunction eq "")
	{
	    if ($datatype =~ /packed_map<(\w+),([\w:]+)>/)
	    {
		print OUTFILE "\t{\n";
		print OUTFILE "\t\tstd::string packedValue;\n";
		print OUTFILE "\t\trow->".&dbIze($cName).".getValue(packedValue);\n";
		print OUTFILE "\t\tArchive::AutoDeltaPackedMap<$1,$2>::pack(data, packedValue);\n";
		print OUTFILE "\t}\n";
	    }
	    elsif (&isNarrowInteger($datatype))
	    {
		# The column is a 32-bit integer and the member is narrower: convert
		# only a value the member can hold, and fail the encode otherwise.
		# A NULL column is an absent value (for example a row older than the
		# column): it encodes as the value a new object gets, with a warning.
		# On the 32-bit server a NULL read left the buffer unchanged (a stale
		# value or a sentinel); this default is a new, deliberate
		# compatibility policy.
		my $nullDefault = &narrowNullDefault($classname, $cName);
		print OUTFILE "\t{\n";
		print OUTFILE "\t\t${datatype} temp = ${nullDefault};\n";
		print OUTFILE "\t\tif (row->".&dbIze($cName).".isNull())\n";
		print OUTFILE "\t\t\tWARNING(true, (\"DatabaseWarning: SwgSnapshot::encode\u${package}${classname}: object %s has a NULL ${rowtype}.".&dbIze($cName)."; encoding the new-object default ${nullDefault}\", objectId.getValueString().c_str()));\n";
		print OUTFILE "\t\telse ";
		print OUTFILE "if (!DB::checkedNarrow(row->".&dbIze($cName).".getValue(), temp, \"${rowtype}.".&dbIze($cName)."\",\n";
		print OUTFILE "\t\t\t[&objectId]() { return \"SwgSnapshot::encode\u${package}${classname}, object \" + objectId.getValueString(); }))\n";
		print OUTFILE "\t\t{\n";
		print OUTFILE "\t\t\tm_encodeRejected = true; // rejects the whole load: see SwgSnapshot::prepareSend\n";
		print OUTFILE "\t\t\treturn false;\n";
		print OUTFILE "\t\t}\n";
		print OUTFILE "\t\tArchive::put(data,temp);\n";
		print OUTFILE "\t}\n";
	    }
	    else
	    {
		print OUTFILE "\t{\n";
		print OUTFILE "\t\t${datatype} temp;\n";
		print OUTFILE "\t\trow->".&dbIze($cName).".getValue(temp);\n";
		print OUTFILE "\t\tArchive::put(data,temp);\n";
		print OUTFILE "\t}\n";
	    }
	}
	else
	{
	    print OUTFILE "\t\t".$persistFunction."\n";
	}
    }
 
    print OUTFILE "\treturn true;\n"; 
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

# Member types narrower than the 32-bit integer column that persists them.
# Encoding converts with a check and decoding widens explicitly. A narrow
# type missing from this list does not compile, because DB::BindableInt32
# exchanges only int32_t.
sub isNarrowInteger
{
    my($datatype)=@_;
    my(%narrow)=("int8" => 1, "uint8" => 1, "int16" => 1, "uint16" => 1,
                 "Postures::Enumerator" => 1); # Postures::Enumerator is an int8
    return exists $narrow{$datatype};
}

# ----------------------------------------------------------------------

# The value a newly created object gives each narrow member (its initializer
# in the object's constructor). A NULL column encodes as this value. A new
# narrow member must be added here, or generation fails.
sub narrowNullDefault
{
    my($classname,$cName)=@_;
    my(%default)=("CreatureObject.m_rank" => "0",                       # CreatureObject(): m_rank(0)
                  "CreatureObject.m_posture" => "Postures::Upright");   # CreatureObject(): m_posture(Postures::Upright)
    my($key)="${classname}.${cName}";
    $key =~ s/\s+$//;
    die "make_packages.pl: no NULL default for narrow member $key" unless exists $default{$key};
    return $default{$key};
}

# ----------------------------------------------------------------------

sub packageCount
{
    my($classname,$package)=@_;
    if ($parent{$classname} eq "")
    {
	return scalar (@{ $packageMembers{"$classname.$package"} }) + 0;
    }
    else
    {
	return scalar (@{ $packageMembers{"$classname.$package"} }) + &packageCount($parent{$classname},$package) + 0;
    }
}

# ----------------------------------------------------------------------

sub dbIze
{ 
    my($name)=@_;
    $name =~ s/m\_//;
    $name =~ s/\s+$//;
    $name =~ s/([a-z])([A-Z])/$1_$2/g;
    $name =~ tr/[A-Z]/[a-z]/;
    $name =~ s/component/cmp/;
    $name =~ s/energy/eng/;
    $name =~ s/hitpoints/hp/;
    $name =~ s/acceleration/acc/;
    $name =~ s/droid_control_device/dcd/;
    $name =~ s/droid_interface_command_speed/droid_if_cmd_speed/;
    $name =~ s/maintenance_requirement/maintenance/;
    return $name;
}

# ----------------------------------------------------------------------

sub makeEncodeSwitcher
{
    my($package)=@_;
    my($classname,$templatetag);
    print OUTFILE "bool SwgSnapshot::encode\u${package}Data(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const\n";
    print OUTFILE "{\n";
    print OUTFILE "\tArchive::ByteStream bs;\n";
    print OUTFILE "\tswitch (typeId)\n";
    print OUTFILE "\t{\n";

	if ($package eq "parentClient")
    {
		$classname="PlayerObject";
		$templatetag="Server${classname}Template::Server${classname}Template_tag";
		print OUTFILE "\tcase ${templatetag}:\n";
		print OUTFILE "\t\tif (!encode\u${package}${classname}(objectId, bs)) return false;\n";
		print OUTFILE "\t\tbreak;\n";
	}
	else
    {
		foreach $classname (sort (keys (%parent)))
		{
			if ($classname eq "ServerObject")
			{
				$templatetag="ServerObjectTemplate::ServerObjectTemplate_tag";
			}
			elsif (!($classname =~ /Object/))
			{
				$templatetag="Server${classname}ObjectTemplate::Server${classname}ObjectTemplate_tag";
			}
			else
			{
				$templatetag="Server${classname}Template::Server${classname}Template_tag";
			}

			print OUTFILE "\tcase ${templatetag}:\n";
			print OUTFILE "\t\tif (!encode\u${package}${classname}(objectId, bs)) return false;\n";
			print OUTFILE "\t\tbreak;\n";
		}
	}
    print OUTFILE "\t}\n";
    print OUTFILE "\n";
    print OUTFILE "\t// test for 0-length message\n";
    print OUTFILE "\tif (bs.getSize() == 0)\n";
    print OUTFILE "\t\treturn true;\n";
    print OUTFILE "\t\n";
    print OUTFILE "\t{\n";
    print OUTFILE "\t\tPROFILER_AUTO_BLOCK_DEFINE(\"new BaselinesMessage\");\n";
    print OUTFILE "\t\tbaselines.push_back(BatchBaselinesMessageData(objectId,typeId,BaselinesMessage::".&baselinesTag($package).",bs));\n";
    print OUTFILE "\t}\n";
    print OUTFILE "\treturn true;\n";
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub baselinesTag
{
    my ($package)=@_;
    return "BASELINES_SHARED" if ($package eq "shared");
    return "BASELINES_SERVER" if ($package eq "server");
    return "BASELINES_CLIENT_SERVER" if ($package eq "client");
    return "BASELINES_FIRST_PARENT_CLIENT_SERVER" if ($package eq "parentClient");
}

# ----------------------------------------------------------------------

sub editDecoder
{
    my($filename)=@_;
    my($classname);

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED DECODER/)
	{
	    foreach $classname (sort (keys (%parent)))
	    {
#	    foreach $classname ("VehicleObject",
#				"TangibleObject" ,
#				"UniverseObject",
#				"ResourceClassObject",
#				"ResourceTypeObject",
#				"CellObject",
#				"InstallationObject",
#				"CreatureObject",
#				"StaticObject",
#				"ServerObject",
#				"ResourcePoolObject",
#				"BuildingObject",
#				"WeaponObject",
#				"PlanetObject")
#	    {
		&makeDecodeFunction($classname,"shared");
		&makeDecodeFunction($classname,"server");
		&makeDecodeFunction($classname,"client");
	    }
	    &makeDecodeFunction("PlayerObject","parentClient");
	
	
	    &makeDecodeSwitcher("server");
	    &makeDecodeSwitcher("shared");
	    &makeDecodeSwitcher("client");
	    &makeDecodeSwitcher("parentClient");

	    while (!/!!!END GENERATED DECODER/)
	    {
		#eat old generated code
		die unless ($_=<INFILE>);
	    }
	    print OUTFILE $_;
	}
    } 

    close (INFILE);
    close (OUTFILE);

    if ($opt_windows == 1)
    {
	system ("copy ${filename}.make_encoder_temporary_file $filename");
	system ("del ${filename}.make_encoder_temporary_file");
    }
    else
    {
	system ("cp ${filename}.make_encoder_temporary_file $filename");
	system ("rm ${filename}.make_encoder_temporary_file");
    }
}

# ----------------------------------------------------------------------

sub makeDecodeFunction
{
    my ($classname,$package)=@_;
    my ($member,$rowtype,$buffer,$index,$cName, $datatype, $addCommand, $persistFunction, $needCloseBracket, $loadFunction);

    print OUTFILE "void SwgSnapshot::decode\u${package}${classname}(NetworkId const & objectId, uint16 index, Archive::ReadIterator &data, bool isBaseline)\n";
    print OUTFILE "{\n";
    print OUTFILE "\tUNREF(isBaseline);\n";
    print OUTFILE "\n";

    $index=&packageCount($parent{$classname},$package);
    if ($index > 0)
    {
	$needCloseBracket=1;

	print OUTFILE "\tif (index < ".$index.")\n";
	print OUTFILE "\t{\n";
	print OUTFILE "\t\tdecode\u${package}${parent{$classname}}(objectId,index,data, isBaseline);\n";
	print OUTFILE "\t}\n";
	print OUTFILE "\telse\n";
	print OUTFILE "\t{\n";
    }
    
    if ($classname eq "ServerObject")
    {
	$rowtype="ObjectBufferRow";
	$buffer="objectTableBuffer";
    }
    else
    {
	if (!($classname =~ /Object$/))
	{
	    $rowtype=$classname."ObjectBufferRow";
	    $buffer=$classname."ObjectBuffer";
	}
	else
	{
	    $rowtype=$classname."BufferRow";
	    $buffer=$classname."Buffer";
	}
    }
    print OUTFILE "\t\tDBSchema::${rowtype} *row=m_\l${buffer}.findRowByIndex(objectId);\n";
    print OUTFILE "\t\tif (row==NULL)\n";
    print OUTFILE "\t\t\trow=m_\l${buffer}.addEmptyRow(objectId);\n";
    print OUTFILE "\n";
    print OUTFILE "\t\tswitch(index)\n";
    print OUTFILE "\t\t{\n";

    $index=&packageCount($parent{$classname},$package);
    foreach $member (@{ $packageMembers{"$classname.$package"} })
    {
	($cName, $datatype, $addCommand, $persistFunction, $loadFunction)=@{ $member };
	
	print OUTFILE "\t\tcase ".($index++).":\n";
	print OUTFILE "\t\t{\n";
	
	if ($loadFunction eq "")
	{
	    if ($datatype =~ /packed_map<(\w+),([\w:]+)>/)
	    {
		print OUTFILE "\t\t\tstd::string packedValue;\n";
		print OUTFILE "\t\t\tArchive::AutoDeltaPackedMap<$1,$2>::unpack(data,packedValue);\n";
		print OUTFILE "\t\t\trow->".&dbIze($cName)."=packedValue;\n";
	    }
	    else
	    {
		print OUTFILE "\t\t\t${datatype} temp;\n";
		print OUTFILE "\t\t\tArchive::get(data,temp);\n";
		if (&isNarrowInteger($datatype))
		{
		    # Widening into the 32-bit column: every value is representable.
		    print OUTFILE "\t\t\trow->".&dbIze($cName)."=static_cast<int32_t>(temp);\n";
		}
		else
		{
		    print OUTFILE "\t\t\trow->".&dbIze($cName)."=temp;\n";
		}
#Hack to catch cell number bug
		if (&dbIze($cName) eq "cell_number")
		{
		    print OUTFILE "\t\t\tWARNING_STRICT_FATAL(temp==-1,(\"Received CellObject %s with cell number -1.\\n\",objectId.getValueString().c_str()));\n";
		}
	    }
	}
	else
	{
	    print OUTFILE "\t\t\t".$loadFunction."\n";
	}

	print OUTFILE "\t\t\tbreak;\n";
	print OUTFILE "\t\t}\n";
    }

    print OUTFILE "\t\tdefault:\n";
    print OUTFILE "\t\t\tDEBUG_REPORT_LOG(true,(\"Fell through cases in decode\u${package}${classname}.  Index is %hu.\\n\",index));\n";
    print OUTFILE "\t\t}\n";
    print OUTFILE "\t}\n" if ($needCloseBracket==1);
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub makeDecodeSwitcher
{
    my($package)=@_;
    my($classname,$templatetag);

    print OUTFILE "void SwgSnapshot::decode\u${package}Data(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline)\n";
    print OUTFILE "{\n";
    print OUTFILE "\tswitch(typeId)\n";
    print OUTFILE "\t{\n";

    if ($package eq "parentClient")
    {
    	$classname="PlayerObject";
    	$templatetag="Server${classname}Template::Server${classname}Template_tag";
    	print OUTFILE "\t\tcase ${templatetag}:\n";
		print OUTFILE "\t\t\tdecode\u${package}${classname}(objectId, index, bs, isBaseline);\n";
		print OUTFILE "\t\t\tbreak;\n";
    }
    else
    {
	    foreach $classname (sort (keys (%parent)))
	    {
		if ($classname eq "ServerObject")
		{
		    $templatetag="ServerObjectTemplate::ServerObjectTemplate_tag";
		}
		elsif(!($classname =~ /Object$/))
		{
		    $templatetag="Server${classname}ObjectTemplate::Server${classname}ObjectTemplate_tag";
		}
		else
		{
		    $templatetag="Server${classname}Template::Server${classname}Template_tag";
		}

		print OUTFILE "\t\tcase ${templatetag}:\n";
		print OUTFILE "\t\t\tdecode\u${package}${classname}(objectId, index, bs, isBaseline);\n";
		print OUTFILE "\t\t\tbreak;\n";
	    }
	}

    print OUTFILE "\t}\n";
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub makeRegisterTags
{
    my($classname,$parent,$templatetag,$buffer);

    print OUTFILE "void SwgSnapshot::registerTags()\n";
    print OUTFILE "{\n";

    foreach $classname (sort (keys (%parent)))
    {
	if ($classname eq "ServerObject")
	{
	    $templatetag="ServerObjectTemplate::ServerObjectTemplate_tag";
	}
	elsif(!($classname =~ /Object$/))
	{
	    $templatetag="Server${classname}ObjectTemplate::Server${classname}ObjectTemplate_tag";
	}
	else
	{
	    $templatetag="Server${classname}Template::Server${classname}Template_tag";
	}

	$parent=$classname;
	while ($parent ne "" && $parent ne "ServerObject")
	{
	    if (!($parent =~ /Object$/))
	    {
		$buffer=$parent."ObjectBuffer";
	    }
	    else
	    {
		$buffer=$parent."Buffer";
	    }
	    print OUTFILE "\tm_\l$buffer.addTag($templatetag);\n";

	    $parent = $parent{$parent};
	}
    }

    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub editLoadObject
{
    my($filename)=@_;
    my($classname);

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED LOADOBJECT/)
	{
	    foreach $classname (sort (keys (%parent)))
	    {
		&makeLoadFunction($classname);
		&makeNewObjectFunction($classname);
	    }

	    &makeLoadSwitcher();
	    &makeNewObjectSwitcher();
	    &makeRegisterTags();
	
	    while (!/!!!END GENERATED LOADOBJECT/)
	    {
		#eat old generated code
		die unless ($_=<INFILE>);
	    }
	    print OUTFILE $_;
	}
    } 

    close (INFILE);
    close (OUTFILE);

    if ($opt_windows == 1)
    {
	system ("copy ${filename}.make_encoder_temporary_file $filename");
	system ("del ${filename}.make_encoder_temporary_file");
    }
    else
    {
	system ("cp ${filename}.make_encoder_temporary_file $filename");
	system ("rm ${filename}.make_encoder_temporary_file");
    }
}

# ----------------------------------------------------------------------

sub makeLoadFunction($classname)
{
    my($classname)=@_;
    my($buffer,$normClassname);

    return if ($classname eq "ServerObject");
    $normClassname=&normalizeClassname($classname);

    $buffer = "m_\l${normClassname}Buffer";

    print OUTFILE "void SwgSnapshot::load${normClassname}(DB::Session *session, NetworkId const & objectId)\n";
    print OUTFILE "{\n";
    print OUTFILE "\tUNREF(session);\n";
    print OUTFILE "\t${buffer}.addEmptyRow(objectId);\n";
#TODO:  this feels like a hack -- find a data-driven way:
    if ($classname eq "CreatureObject")
    {
	print OUTFILE "\tloadAttributes(session,objectId);\n";
	print OUTFILE "\tloadSkills(session,objectId);\n";
	print OUTFILE "\tloadCommands(session,objectId);\n";
    }
    if ($parent{$classname} ne "" && $parent{$classname} ne "ServerObject")
    {
	print OUTFILE "\tload$parent{$classname}(session,objectId);\n";
    }
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub makeNewObjectFunction($classname)
{
    my($classname)=@_;
    my($buffer,$normClassname);

    return if ($classname eq "ServerObject");
    $normClassname=&normalizeClassname($classname);

    $buffer = "m_\l${normClassname}Buffer";

    print OUTFILE "void SwgSnapshot::new${normClassname}(NetworkId const & objectId)\n";
    print OUTFILE "{\n";
    print OUTFILE "\t${buffer}.addEmptyRow(objectId);\n";
#TODO:  attributes if creature, etc. -- are they really needed (will get created by the baselines automatically?)
    if ($parent{$classname} ne "" && $parent{$classname} ne "ServerObject")
    {
	print OUTFILE "\tnew$parent{$classname}(objectId);\n";
    }
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub makeLoadSwitcher()
{
    my($classname,$templatename);

    print OUTFILE "void SwgSnapshot::loadObject(DB::Session *session,const Tag &typeId, const NetworkId &objectId)\n";
    print OUTFILE "{\n";
    print OUTFILE "\tswitch(typeId)\n";
    print OUTFILE "\t{\n";

    foreach $classname (sort keys(%parent))
    {
	next if ($classname eq "ServerObject");
	$classname=&normalizeClassname($classname);
	$templatename="Server${classname}Template";

	print OUTFILE "\t\tcase ${templatename}::${templatename}_tag:\n";
	print OUTFILE "\t\t\tload${classname}(session,objectId);\n";
	print OUTFILE "\t\t\tbreak;\n";
	print OUTFILE "\n";
    }

    print OUTFILE "\t\tdefault:\n";
    print OUTFILE "\t\t\tWARNING(true,(\"Attempt to load object %s from the database.  It has unknown type %i.\",objectId.getValueString().c_str(),typeId));\n";
    print OUTFILE "\t}\n";
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub makeNewObjectSwitcher()
{
    my($classname,$templatename);

    print OUTFILE "void SwgSnapshot::newObject(NetworkId const & objectId, uint32 templateId, Tag typeId)\n";
    print OUTFILE "{\n";
    print OUTFILE "\tm_objectTableBuffer.newObject(objectId, templateId, typeId);\n";
    print OUTFILE "\tswitch(typeId)\n";
    print OUTFILE "\t{\n";

    foreach $classname (sort keys(%parent))
    {
	next if ($classname eq "ServerObject");
	$classname=&normalizeClassname($classname);
	$templatename="Server${classname}Template";

	print OUTFILE "\t\tcase Server${classname}Template::Server${classname}Template_tag:\n";
	print OUTFILE "\t\t\tnew${classname}(objectId);\n";
	print OUTFILE "\t\t\tbreak;\n";
	print OUTFILE "\n";
    }

    print OUTFILE "\t\tdefault:\n";
    print OUTFILE "\t\t\tWARNING(true,(\"Attempt to create object %s.  It has unknown type %i.\",objectId.getValueString().c_str(),typeId));\n";
    print OUTFILE "\t}\n";
    print OUTFILE "}\n";
    print OUTFILE "\n";
}

# ----------------------------------------------------------------------

sub normalizeClassname()
{
    my ($classname)=@_;
    if ($classname =~ /Object$/)
    {
	return $classname;
    }
    else
    {
	return $classname."Object";
    }
}


# ======================================================================
