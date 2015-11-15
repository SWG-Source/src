#!/usr/bin/perl

#
# This generates gueries to call stored procedures in a PL/SQL package.
#
# For now, speciallized to just make the xxxObjectQuery classes.
# Eventually may expand this to make a query to run any arbirary function
#

use Getopt::Long;

&main;

# ======================================================================

sub main
{
    &GetOptions("plsql=s","header=s","source=s","windows");
    
    if ($opt_plsql eq "")
    {
	print "Usage:\n";
	print "--plsql <filename>    Specify the file to read the PL/SQL package from.\n";
	print "--source <filename>   Write queries to call the functions in the package.\n";
	print "--header <filename>   Write the declarations of the query classes to this file.\n";
	print "--windows             Use this option when running under Windows.\n";
	die;
    }

    open (INFILE,$opt_plsql);
    while (<INFILE>)
    {
	last if /end;/;
	&parseProc if /procedure save\_(\w+)\_obj/;
    }
    close (INFILE);

#    foreach $classname (sort keys %procParameters)
#    {
#	print "$classname:  ";
#	print join (",",@{ $procParameters{$classname} })."\n";
#    }

    if ($opt_header)
    {
	&writeHeader($opt_header);
    }

    if ($opt_source)
    {
	&writeSource($opt_source);
    }
}

# ----------------------------------------------------------------------

sub parseProc
{
#   print $_;
    my ($tableName,$params,@params, $datatype, @datatype);
    /procedure save\_(\w+)\_obj\s*\(([^\)]*)\);\s*$/;
    $tableName=$1;
 
    $params=$2;
#   print $params."\n";
    
    @params = split (/,/,$params);
    for ($i=0; $i!=@params; ++$i)
    {
	$params[$i] =~ s/(\w+)\s+(\w+)/$1/;
	$datatype[$i]=$2;
	$params[$i] =~ s/\s+//g;
	}
#	print "Params: ".join (":",@params)."\n";
#	print "Datatypes: ".join (":",@datatype)."\n";

    $procParameters{$tableName} = [@params];
	$procDatatypes{$tableName} = [@datatype];
	
}

# ----------------------------------------------------------------------

sub writeHeader
{
    my($filename)=@_;
    my($newText,$tablename);

    foreach $tablename (sort keys %procParameters)
    { 
	$classname = $tablename;
	$classname =~ s/harvester\_inst/harvester\_installation/g;
	$classname =~ s/manufacture\_inst/manufacture\_installation/g;
	$classname =~ s/manf\_schematic/manufacture\_schematic/g;
	$classname =~ s/\_/\\u/g;
	eval("\$classname=\"\\u".$classname."\";");
	$rowname = "DBSchema::${classname}ObjectRow";
	$classname.="ObjectQuery";
	
	$newText.="class ${classname} : public DatabaseProcessQuery\n";
	$newText.="{\n";
	$newText.="\t${classname}(const ${classname}&);\n";
	$newText.="\t${classname}& operator= (const ${classname}&);\n";
	$newText.="public:\n";
	$newText.="\t${classname}();\n";
	$newText.="\n";
	$newText.="\tvirtual bool bindParameters ();\n";
	$newText.="\tvirtual bool bindColumns    ();\n";
	$newText.="\tvirtual void getSQL(std::string &sql);\n";
	$newText.="\n";
	$newText.="\tbool setupData(DB::Session *session);\n";
	$newText.="\tbool addData(const DB::Row *_data);\n";
	$newText.="\tvoid clearData();\n";
	$newText.="\tvoid freeData();\n";
	$newText.="\n";
	$newText.="\tint getNumItems() const;\n";
	$newText.="\n";
	$newText.="private:\n";
	$i =0;
	foreach $param (@{ $procParameters{$tablename} })
	{   
	    $param =~ s/^p\_//;
		if ($param ne "chunk_size") {
			$newText.=BindType($procDatatypes{$tablename}[$i], $param);
		}

		$i++;
	}
	$newText.="\tDB::BindableLong\t\t\t m_numItems;\n";
	$newText.="};\n";
	$newText.="\n";

	$newText.="class ${classname}Select : public DB::Query\n";
	$newText.="{\n";
	$newText.="public:\n";
	$newText.="\t${classname}Select(const std::string &schema);\n";
	$newText.="\n";
	$newText.="\tvirtual bool bindParameters ();\n";
	$newText.="\tvirtual bool bindColumns    ();\n";
	$newText.="\tvirtual void getSQL         (std::string &sql);\n";
	$newText.="\n";
	$newText.="\tconst std::vector<${rowname}> & getData() const;\n";
	$newText.="\n";
	$newText.="protected:\n";
	$newText.="\tvirtual QueryMode getExecutionMode() const;\n";
	$newText.="\n";
	$newText.="private:\n";
	$newText.="\n";
	$newText.="\tstd::vector<${rowname}> m_data;\n";
	$newText.="\tconst std::string m_schema;\n";
	$newText.="\n";
	$newText.="\t${classname}Select            (const ${classname}Select&);\n";
	$newText.="\t${classname}Select& operator= (const ${classname}Select&);\n";
	$newText.="};\n";
	$newText.="\n";
    }

    &editFile($filename,"QUERYDECLARATIONS",$newText);
}

# ----------------------------------------------------------------------

sub editFile
{
    my($filename,$tag,$newtext)=@_;  

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED $tag/)
	{
	    print OUTFILE $newtext;

	    while (!/!!!END GENERATED $tag/)
	    {
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
sub BindType
{
	my ($game_type,$param)=@_;

	SWITCH: {

#		print "Game type: $game_type\n";
		if ($game_type eq "VAOFSTRING") {return "\tDB::BindableVarrayString\t m_$param\s;\n"; last SWITCH; }
		if ($game_type eq "VAOFLONGSTRING") {return "\tDB::BindableVarrayString\t m_$param\s;\n"; last SWITCH; }
		if ($game_type eq "VAOFNUMBER") {return "\tDB::BindableVarrayNumber\t m_$param\s;\n"; last SWITCH; }
		print "Warning unknown type: ".$game_type."\n";
		return "VAUNKNOWN";

	}

}


# ----------------------------------------------------------------------
sub BindableTypeConvert
{
	my ($bind_type,$param)=@_;

	SWITCH: {

		
		if ($bind_type eq "VAOFNUMBER") {return ".getValue()"; last SWITCH; }
		if ($bind_type eq "VAOFSTRING") {return ".getValueASCII()"; last SWITCH; }
		if ($bind_type eq "VAOFLONGSTRING") {return ".getValueASCII()"; last SWITCH; }
		print "Warning unknown type: ".$bind_type."\n";
		return $bind_type;

	}

}

# ----------------------------------------------------------------------

sub writeSource
{
    my($filename)=@_;
    my($newText,$tablename,$param,@params,$datatype,$column,$insertText,$indx);

    foreach $tablename (sort keys %procParameters)
    { 
	$classname = $tablename;
	$classname =~ s/harvester\_inst/harvester\_installation/g;
	$classname =~ s/manufacture\_inst/manufacture\_installation/g;
	$classname =~ s/manf\_schematic/manufacture\_schematic/g;
	$classname =~ s/\_/\\u/g;
	eval("\$classname=\"\\u".$classname."ObjectQuery\";");
	$rowname = $classname;
	$rowname =~ s/Query/Row/;
	$bufferRowname = $rowname;
	$bufferRowname =~ s/ObjectRow/ObjectBufferRow/;

	$newText.="${classname}::${classname}() :\n";
	$newText.="\tDatabaseProcessQuery(new ${rowname}),\n";
	$newText.="\tm_numItems(0)\n";
	$newText.="{\n";
	$newText.="}\n";
	$newText.="\n";

# SETUPDATA

	$newText.="bool ${classname}::setupData(DB::Session *session)\n";
	$newText.="{\n";
	$newText.="\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\tcase mode_INSERT:\n";

	$i =0;
	foreach $param (@{ $procParameters{$tablename} })
	{   
	    $param =~ s/^p\_//;
	    if ($param ne "chunk_size") {
		$sizeLimit= "";
		$sizeLimit=", 1000" if ($procDatatypes{$tablename}[$i] eq "VAOFSTRING");
		$sizeLimit=", 4000" if ($procDatatypes{$tablename}[$i] eq "VAOFLONGSTRING");

		$newText.="\t\t\tif (!m_$param\s.create(session, \"$procDatatypes{$tablename}[$i]\", DatabaseProcess::getInstance().getSchema()$sizeLimit)) return false;\n";
	    }
	    $i++;

	}
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";
	$i=0;
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
	    if ($param eq "object_id") { 
		$newText.="\t\t\tif (!m_$param\s.create(session, \"$procDatatypes{$tablename}[$i]\", DatabaseProcess::getInstance().getSchema(),1000)) return false;\n";
	    }
	    $i++;

	}

	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\treturn true;\n";
	$newText.="}\n";
	$newText.="\n";


# ADD DATA

	$newText.="bool ${classname}::addData(const DB::Row *_data)\n";
	$newText.="{\n";
	$newText.="\tconst $bufferRowname *myData=safe_cast<const $bufferRowname*>(_data);";
	$newText.="\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\tcase mode_INSERT:\n";

	$i =0;
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param ne "chunk_size") {
    	if ($procDatatypes{$tablename}[$i] eq "VAOFNUMBER")
				{ $newText.="\t\t\tif (!m_$param\s.push_back(myData->$param\.isNull(), myData->$param".BindableTypeConvert($procDatatypes{$tablename}[$i], $param).")) return false;\n";
    		}
    	else
				{ $newText.="\t\t\tif (!m_$param\s.push_back(myData->$param".BindableTypeConvert($procDatatypes{$tablename}[$i], $param).")) return false;\n";
    		}
		}
		$i++;
	}

	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";

	$i =0;
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param eq "object_id") { 
    	if ($procDatatypes{$tablename}[$i] eq "VAOFNUMBER")
				{ $newText.="\t\t\tif (!m_$param\s.push_back(myData->$param\.isNull(), myData->$param".BindableTypeConvert($procDatatypes{$tablename}[$i], $param).")) return false;\n";
    		}
    	else
				{ $newText.="\t\t\tif (!m_$param\s.push_back(myData->$param".BindableTypeConvert($procDatatypes{$tablename}[$i], $param).")) return false;\n";
    		}
		}
		$i++;
	}

	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\n";
	$newText.="\tm_numItems=m_numItems.getValue() + 1;\n";
	$newText.="\treturn true;\n";
	$newText.="}\n";
	$newText.="\n";


# GETNUMITEMS

	$newText.="int ${classname}::getNumItems() const\n";
	$newText.="{\n";
	$newText.="\treturn m_numItems.getValue();\n";
	$newText.="}\n";
	$newText.="\n";


# CLEARDATA

	$newText.="void ${classname}::clearData()\n";
	$newText.="{\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\tcase mode_INSERT:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param ne "chunk_size") {
			$newText.="\t\t\tm_$param\s.clear();\n";
		}
	}
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param eq "object_id") { 
			$newText.="\t\t\tm_$param\s.clear();\n";
		}
	}
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\n";
	$newText.="\tm_numItems=0;\n";
	$newText.="}\n";
	$newText.="\n";

# FREEDATA

	$newText.="void ${classname}::freeData()\n";
	$newText.="{\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\tcase mode_INSERT:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param ne "chunk_size") {
			$newText.="\t\t\tm_$param\s.free();\n";
		}
	}
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param eq "object_id") { 
			$newText.="\t\t\tm_$param\s.free();\n";
		}
	}
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\n";
	$newText.="}\n";
	$newText.="\n";

# BINDPARAMETERS

	$newText.="bool ${classname}::bindParameters()\n";
	$newText.="{\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\tcase mode_INSERT:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param ne "chunk_size") {
			$newText.="\t\t\tif (!bindParameter(m_$param\s)) return false;\n";		
		}
	}
	$newText.="\t\t\tif (!bindParameter(m_numItems)) return false;\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    $param =~ s/^p\_//;
		if ($param eq "object_id") { 
			$newText.="\t\t\tif (!bindParameter(m_$param\s)) return false;\n";
		}
	}
	$newText.="\t\t\tif (!bindParameter(m_numItems)) return false;\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\treturn true;\n";
	$newText.="\n";
	$newText.="}\n";
	$newText.="\n";

# BINDCOLUMNS

	$newText.="bool ${classname}::bindColumns()\n";
	$newText.="{\n";
	$newText.="\treturn true;\n";
	$newText.="}\n";
	$newText.="\n";


# GETSQL

	$newText.="void ${classname}::getSQL(std::string &sql)\n";
	$newText.="{\n";
	$newText.="\tswitch(mode)\n";
	$newText.="\t{\n";
	$newText.="\t\tcase mode_UPDATE:\n";
	$newText.="\t\t\tsql=std::string(\"begin \")+DatabaseProcess::getInstance().getSchemaQualifier()+\"persister.save_${tablename}_obj (:".join(", :",@{ $procParameters{$tablename} })."); end;\";\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_INSERT:\n";
	$newText.="\t\t\tsql=std::string(\"begin \")+DatabaseProcess::getInstance().getSchemaQualifier()+\"persister.add_${tablename}_obj (:".join(", :",@{ $procParameters{$tablename} })."); end;\";\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_SELECT:\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tcase mode_DELETE:\n";
	$newText.="\t\t\tsql=std::string(\"begin \")+DatabaseProcess::getInstance().getSchemaQualifier()+\"persister.remove_${tablename}_obj (:object_id, :chunk_size ); end;\";\n";
	$newText.="\t\t\tbreak;\n";
	$newText.="\n";
	$newText.="\t\tdefault:\n";
	$newText.="\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
	$newText.="\t}\n";
	$newText.="\n";
	$newText.="}\n";
	$newText.="\n";


# SELECT query

	$newText.="${classname}Select::${classname}Select(const std::string &schema) :\n";
	$newText.="\tm_data(ms_fetchBatchSize),\n";
	$newText.="\tm_schema(schema)\n";
	$newText.="{\n";
	$newText.="}\n";
	$newText.="\n";
	$newText.="bool ${classname}Select::bindParameters ()\n";
	$newText.="{\n";
	$newText.="\treturn true;\n";
	$newText.="}\n";
	$newText.="\n";
	$newText.="bool ${classname}Select::bindColumns()\n";
	$newText.="{\n";
	$newText.="\tsize_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));\n";
	$newText.="\tsetColArrayMode(skipSize, ms_fetchBatchSize);\n";
	$newText.="\n";	
	$newText.="\tif (!bindCol(m_data[0].object_id)) return false;\n";
	foreach $param (@{ $procParameters{$tablename} })
	{
	    #We assume the select proc returns the same columns as the update proc's parameters
	    $param =~ s/^p\_//;
	    next if $param eq "object_id";
		next if $param eq "chunk_size";
	    $newText.="\tif (!bindCol(m_data[0].$param)) return false;\n";
	}
	$newText.="\treturn true;\n";
	$newText.="}\n";
	$newText.="\n";
	$newText.="const std::vector<${rowname}> & ${classname}Select::getData() const\n";
	$newText.="{\n";
	$newText.="\treturn m_data;\n";
	$newText.="}\n";
	$newText.="\n";
	$newText.="void ${classname}Select::getSQL(std::string &sql)\n";
	$newText.="{\n";
	$newText.="\t\t\tsql=std::string(\"begin :result := \")+m_schema+\"loader.load_${tablename}_object; end;\";\n";
	$newText.="}\n";
	$newText.="\n";
	$newText.="DB::Query::QueryMode ${classname}Select::getExecutionMode() const\n";
	$newText.="{\n";
	$newText.="\treturn MODE_PLSQL_REFCURSOR;\n";
	$newText.="}\n";
	$newText.="\n";
    }

    &editFile($filename,"QUERYDEFINITIONS",$newText);
#    print $newText;
}

# ======================================================================
