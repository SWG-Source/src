#!/usr/bin/perl

# This generates a C++ header file from the DDL for the database tables.
# The header file contains structs to represent each database table's rows.
#
# Note that the DDL file must be formatted like this:
# create table <table>
# (
#    <column> <datatype>,
#     ...
# );
#
# It is recommended that the header file be called Schema.h

use Getopt::Long;

&main;

# ======================================================================

sub main
{
    &GetOptions("ddldirectory:s","output:s","windows");
    
    if (($opt_output eq "") || ($opt_ddldirectory eq ""))
    {
	print "Usage:\n";
	print "--ddldirectory <pathname>       Specify the directory in which the create table scripts live.  They are assumed to be named <tablename>.tab\n";
	print "--output <filename>             Specify the file to write (usually Schema.h)\n";
	print "--objectqueryheader <filename>  Write the declarations for the xxxObjectQuery classes\n";
	print "--windows                       Use this option when running under Windows.\n";
	die;
    }
    
    $opt_ddldirectory =~ s/\/$//;

    if ($opt_windows == 1)
    {
	open (FILELIST,"c:\cygwin\bin\bash -c \"c:\cygwin\bin\ls ${opt_ddldirectory}/*.tab\"|"); #dir /f /b ${opt_ddldirectory}\\*.tab|");
    }
    else
    {
	open (FILELIST,"ls ${opt_ddldirectory}/*.tab|");
    }
    while (<FILELIST>)
    {
	chop;
	$filename = $_;
	&parseTable($filename);
    }
    close(FILELIST);

    &output($opt_output);
}

# ----------------------------------------------------------------------

sub parseTable
{
    my($filename)=@_;
    my($tablename,$forcetype,$nobind,$copy,$init);

    print "$filename\n";
    open (INFILE, $filename);
    while (<INFILE>)
    {
	if (/create table (\w+)/ && (!/NO_IMPORT/))
	{
	    $tablename = $1;
	    $tablename =~ s/s$//; #remove plural
	    $tablename =~ s/manufacture_inst/manufacture_installation/g; # C name was too long for Oracle
	    $tablename =~ s/manf_schematic/manufacture_schematic/g; # C name was too long for Oracle
	    $tablename =~ s/_/\\u/g;
	    $tablename = "\u$tablename";
	    eval("\$tablename = \"".$tablename."\";"); #Gotta love that PERL.  (This converts _[a-z] to _[A-Z])
	    $decl{$tablename}="struct $tablename"."Row : public DB::Row\n{\n";
	    $buffer{$tablename}="struct $tablename"."BufferRow : public DB::Row\n{\n";

	    die unless ($_=<INFILE>); # eat "(";
	    die unless ($_=<INFILE>);
	    while (!($_ =~ /^\)/))
	    {
		chop;
		if (/\-\-\s*BIND_AS\(([^\)]*)\)/)
		{
		    $forcetype=$1;
		}
		else
		{
		    $forcetype="";
		}
		if (/\-\-\s*NO_BIND/)
		{
		    $nobind=1;
		}
		else
		{
		    $nobind=0;
		}
		s/\s*\-\-.*//;
		s/,\s*$//;
		if (($nobind==0) && (/^\s*(\w*)\s+(.*)$/) && !(/primary key/) && !(/foreign key/))
		{
		    ($name,$type)=($1,$2);
		    if ($forcetype eq "")
		    {
			$decl{$tablename}.="\t".&translateType($type);
			$buffer{$tablename}.="\t".&translateBufferType($type);
		    }
		    else
		    {
			$decl{$tablename}.="\t".$forcetype;
			$buffer{$tablename}.="\t".$forcetype;
		    }
		    $decl{$tablename}.=" $name;\n";
		    $buffer{$tablename}.=" $name;\n";
		    $copy.= "\t\t$name(rhs.$name),\n";
		    if ($forcetype eq "")
		    {
			$init.= "\t\t$name(".&constructorParams($type)."),\n";
		    }
		    else
		    {
			$init.= "\t\t$name(),\n";
		    }
		}
		$_=<INFILE>;
	    }   
	    if ($tablename eq "Object")
	    {
		$decl{$tablename}.="\n\tDB::BindableLong container_level;\n";
	    }

	    $decl{$tablename}.="\n\tvirtual void copy(const DB::Row &rhs)\n";
	    $decl{$tablename}.="\t{\n";
	    $decl{$tablename}.="\t\t*this = dynamic_cast<const ".$tablename."Row&>(rhs);\n";
	    $decl{$tablename}.="\t}\n";
	    if ($tablename =~ /Object$/)
	    {
		$decl{$tablename}.="\n\tvoid setPrimaryKey(const NetworkId &keyValue)\n";
		$decl{$tablename}.="\t{\n";
		$decl{$tablename}.="\t\tobject_id.setValue(keyValue);\n";
		$decl{$tablename}.="\t}\n";

		$decl{$tablename}.="\n\tNetworkId getPrimaryKey() const\n";
		$decl{$tablename}.="\t{\n";
		$decl{$tablename}.="\t\treturn object_id.getValue();\n";
		$decl{$tablename}.="\t}\n";
	    }
	    if ($tablename eq "Message")
	    {
		$decl{$tablename}.="\n\tvoid setPrimaryKey(const NetworkId &keyValue)\n";
		$decl{$tablename}.="\t{\n";
		$decl{$tablename}.="\t\tmessage_id.setValue(keyValue);\n";
		$decl{$tablename}.="\t}\n";
	    }
	    $decl{$tablename}.="};\n";

	    $buffer{$tablename}.="\n\t${tablename}BufferRow() :\n";
	    $init =~ s/,$//;
	    $buffer{$tablename}.=$init;
	    $buffer{$tablename}.="\t{\n";
	    $buffer{$tablename}.="\t}\n";

	    $buffer{$tablename}.="\n\t${tablename}BufferRow(const ${tablename}Row & rhs) :\n";
	    $copy =~ s/,$//;
	    $buffer{$tablename}.= $copy;
	    $buffer{$tablename}.="\t{\n";
	    $buffer{$tablename}.="\t}\n";

	    $buffer{$tablename}.="\n\tvirtual void copy(const DB::Row &rhs)\n";
	    $buffer{$tablename}.="\t{\n";
	    $buffer{$tablename}.="\t\t*this = dynamic_cast<const ".$tablename."BufferRow&>(rhs);\n";
	    $buffer{$tablename}.="\t}\n";
	    if ($tablename =~ /Object$/)
	    {
		$buffer{$tablename}.="\n\tvoid setPrimaryKey(const NetworkId &keyValue)\n";
		$buffer{$tablename}.="\t{\n";
		$buffer{$tablename}.="\t\tobject_id.setValue(keyValue);\n";
		$buffer{$tablename}.="\t}\n";

		$buffer{$tablename}.="\n\tNetworkId getPrimaryKey() const\n";
		$buffer{$tablename}.="\t{\n";
		$buffer{$tablename}.="\t\treturn object_id.getValue();\n";
		$buffer{$tablename}.="\t}\n";
	    }
	    if ($tablename eq "Message")
	    {
		$buffer{$tablename}.="\n\tvoid setPrimaryKey(const NetworkId &keyValue)\n";
		$buffer{$tablename}.="\t{\n";
		$buffer{$tablename}.="\t\tmessage_id.setValue(keyValue);\n";
		$buffer{$tablename}.="\t}\n";
	    }
	    $buffer{$tablename}.="};\n";
	}
    }

    close(INFILE);
}

# ----------------------------------------------------------------------

sub translateType
{
    my($dbtype)=@_;
  SWITCH:
    {
	if ($dbtype eq "int") { $ctype="DB::BindableLong"; last SWITCH; }
	if ($dbtype eq "number") { $ctype="DB::BindableLong"; last SWITCH; }
	if ($dbtype eq "float") { $ctype="DB::BindableDouble"; last SWITCH; }
	if ($dbtype eq "date") { $ctype="DB::BindableTimestamp"; last SWITCH; }
	if ($dbtype eq "char(1)") { $ctype="DB::BindableBool"; last SWITCH; }
	if ($dbtype =~ /^varchar(2)?\((\d+)\)/) { $ctype="DB::BindableString<$2>"; last SWITCH; }
	die "Datatype $dbtype not handled.";
    }
    
    $ctype
}

# ----------------------------------------------------------------------

sub translateBufferType
{
    my($dbtype)=@_;
  SWITCH:
    {
	if ($dbtype eq "int") { $ctype="DB::BindableLong"; last SWITCH; }
	if ($dbtype eq "number") { $ctype="DB::BindableLong"; last SWITCH; }
	if ($dbtype eq "float") { $ctype="DB::BindableDouble"; last SWITCH; }
	if ($dbtype eq "date") { $ctype="DB::BindableTimestamp"; last SWITCH; }
	if ($dbtype eq "char(1)") { $ctype="DB::BindableBool"; last SWITCH; }
	if ($dbtype =~ /^varchar(2)?\((\d+)\)/) { $ctype="DB::BufferString"; last SWITCH; }
	die "Datatype $dbtype not handled.";
    }
    
    $ctype
}


# ----------------------------------------------------------------------

sub constructorParams
{
    my($dbtype)=@_;
    my($params);
    if ($dbtype =~ /^varchar(2)?\((\d+)\)/)
    {
	$params=$2;
    }
    
    return $params;
}

# ----------------------------------------------------------------------

sub output
{
    my ($filename) = @_;

    open (INFILE,"$filename");
    open (OUTFILE,">${filename}.make_encoder_temporary_file");

    while (<INFILE>)
    {
	print OUTFILE $_;
	if (/!!!BEGIN GENERATED DECLARATIONS/)
	{

	    foreach $tablename (sort keys (%decl))
	    {
		print OUTFILE $decl{$tablename};
		print OUTFILE "\n";
		print OUTFILE $buffer{$tablename};
		print OUTFILE "\n";
	    }	   
	    
	    while (!/!!!END GENERATED DECLARATIONS/)
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
	#system ("copy ${filename}.make_encoder_temporary_file $filename");
	system ("copy d:\\whitengold\\src\\game\\server\\application\\SwgDatabaseServer\\src\\shared\\generated\\Schema_h.template $filename");
	system ("del ${filename}.make_encoder_temporary_file");
    }
    else
    {
	system ("cp ${filename}.make_encoder_temporary_file $filename");
	system ("rm ${filename}.make_encoder_temporary_file");
    }
}
