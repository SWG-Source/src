#!/usr/bin/perl

# This auto-gens some of the more boring code for dealing with 
# database objects.  The generated code needs to be cut-and
# pasted into the appropriate files.  The idea is to save
# typing and reduce errors, not to fully automate the system.

open (TYPEMAP,"typemap.def");
while ($_=<TYPEMAP>)
{
    chop;
    ($c,$db,$cast) = split /\t/;
    $typemap{$c}=$db;
    if ($cast ne "")
    {
	$cast{$c}=1;
    }
}
close (TYPEMAP);

open (BINDMAP,"bindmap.def");
while ($_=<BINDMAP>)
{
    chop;
    ($db,$bindable) = split /\t/;
    $bindablemap{$db}=$bindable;
}
close (BINDMAP);

while (<>)
{
    if(/\/\/.*BPM/)
    {
	&parseBPM();
    }
}

open (SQL,">sql.auto");
open (SCHEMA,">Schema.h.auto");
open (QUERY,">ObjectQueries.cpp.auto");
open (QUERYH,">ObjectQueries.h.auto");
open (MAKEPACKAGE,">MakeDBPackage.cpp.auto");
open (PACKAGEH,">DatabasePackage.h.auto");
open (SAVEPACKAGE,">DatabasePackage.cpp.auto");

# Print out the tables
foreach $tablename (sort keys %columns)
{
    # build a list of columns, including those from inherited classes
    $col="";
    for ($dep=$depend{$tablename}; $dep ne ""; $dep=$depend{$dep})
    {
	$col=$col.$columns{$dep};
    }

    $col=$col.$columns{$tablename};

    ########################################################
    # print out the table creation script
    ########################################################

    print SQL "create table ".&dbIzeName($tablename)."s\n(\n";
    @tabledef = ();
    push (@tabledef, "\tobject_id integer");
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;
	if ($typemap{$type} eq "")
	{
#	    push (@tabledef, "--\t".$name." ".$type)
	}
	else
	{
	    push (@tabledef, "\t".&dbIzeName($name)." ".$typemap{$type});
	}
    }

    print SQL join (",\n",@tabledef);
    print SQL "\n);\n\n";

    ########################################################
    # print out the row object
    ########################################################

    print SCHEMA "struct ".$tablename."Row : public DB::Row\n{\n";
    print SCHEMA "\tDB::BindableLong object_id;\n";
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;
	if ($typemap{$type} ne "")
	{
	    print SCHEMA "\tDB::".$bindablemap{$typemap{$type}}." ".&dbIzeName($name).";\n";
	}
    }
    print SCHEMA "};\n\n";

    ########################################################
    #print out the query class declaration
    ########################################################

    print QUERYH "class ".$tablename."Query : public DB::ModeQuery\n{\n";
    print QUERYH "\t".$tablename."Query(const ".$tablename."Query&); // disable\n";
    print QUERYH "\t".$tablename."Query& operator=(const ".$tablename."Query&); // disable\n";
    print QUERYH "public:\n";
    print QUERYH "\t".$tablename."Query() {}\n\n";
    print QUERYH "\tbool bind();\n";
    print QUERYH "\tchar *getSQL();\n";
    print QUERYH "};\n\n";

    ########################################################
    #print out the bind function
    ########################################################

    $count=1;
    $bind="bool ".$tablename."Query::bind()\n{\n";
    $bind=$bind."\t".$tablename."Row *myData=dynamic_cast<".$tablename."Row*>(data);\n";
    $bind=$bind."\tswitch (mode)\n\t{\n";

    $bindObjId="\t\t\tif (!myData->object_id.bindParameter(1,this)) return false;\n";

    $updSection="\t\tcase mode_UPDATE:\n\t\tcase mode_INSERT:\n";
    $selSection="\t\tcase mode_SELECT:\n".$bindObjId."\n";
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;
	if ($typemap{$type} ne "")
	{
	    $updSection=$updSection."\t\t\tif (!myData->".&dbIzeName($name).".bindParameter(".$count.",this)) return false;\n";
	    $selSection=$selSection."\t\t\tif (!myData->".&dbIzeName($name).".bindCol(".$count.",this)) return false;\n";
	    $count++;
	}
    }
    $updSection=$updSection."\n\t\t\tif (!myData->object_id.bindParameter($count,this)) return false;\n";


    $bind=$bind.$updSection."\t\t\tbreak;\n\n".$selSection."\t\t\tbreak;\n\n";
    $bind=$bind."\t\tcase mode_DELETE:\n".$bindObjId."\t\t\tbreak;\n\n";
    $bind=$bind."\t\tdefault:\n\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";
    $bind=$bind."\t}\n\treturn true;\n}\n\n";
    print QUERY $bind;  #fix object_id -- should be last for update,insert


    ########################################################
    # Print out the SQL
    ########################################################

    @columnNames=();
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;
	if ($typemap{$type} ne "")
	{
	    push @columnNames,&dbIzeName($name);
	}
    }

    print QUERY "char *".$tablename."Query::getSQL()\n{\n\tswitch(mode)\n\t{\n";
    print QUERY "\t\tcase mode_UPDATE:\n";
    print QUERY "\t\t\treturn (\"update ".&dbIzeName($tablename)."s set ";
    print QUERY join("=?, ",@columnNames)."=? where object_id=?\");\n";

    print QUERY "\t\tcase mode_INSERT:\n";
    print QUERY "\t\t\treturn (\"insert into ".&dbIzeName($tablename)."s (";
    print QUERY join(", ",@columnNames).", object_id) values (";
    foreach $temp (@columnNames) { print QUERY "?, ";}
    print QUERY "?)\");\n";

    print QUERY "\t\tcase mode_DELETE:\n";
    print QUERY "\t\t\treturn (\"delete from ".&dbIzeName($tablename)."s where object_id=?\");\n";

    print QUERY "\t\tcase mode_SELECT:\n";
    print QUERY "\t\t\treturn (\"select ";
    print QUERY join(", ",@columnNames);
    print QUERY " from ".&dbIzeName($tablename)."s where object_id=?\");\n";

    print QUERY "\t\tdefault:\n";
    print QUERY "\t\t\tDEBUG_FATAL(true,(\"Bad query mode.\"));\n";

    print QUERY "\t}\n\treturn 0; // this is unreachable but here to avoid compiler warning\n}\n\n";

    ########################################################
    # Print out the Package declaration
    ########################################################

    print PACKAGEH "class ".$tablename."Package : public ServerObjectPackage\n{\n";
    print PACKAGEH "public:\n";
    print PACKAGEH "\t".$tablename."Package() {}\n\n";

    print PACKAGEH "\tvirtual void finishLoad(Snapshot *snap, DB::Session *ses);\n";
    print PACKAGEH "\tvirtual void save(Snapshot *snap, DB::Session *ses);\n";

    print PACKAGEH "private:\n";
    print PACKAGEH "\t".$tablename."Package(const ".$tablename."Package&); //disable\n";
    print PACKAGEH "\t".$tablename."Package& operator=(const ".$tablename."Package&); // disable\n\n";
    print PACKAGEH "\tDBSchema::".$tablename."Row data;\n\n";

    print PACKAGEH "\tfriend class ::".$tablename.";\n};\n\n";

    ########################################################
    # Print out the CreatePacakge functions
    ########################################################

    print MAKEPACKAGE "DBPackage::ServerObjectPackage* ".$tablename."::MakeDBPackage(void) const\n{\n";
    print MAKEPACKAGE "\tDBPackage::".$tablename."Package *buffer=new DBPackage::".$tablename."Package;\n";
    print MAKEPACKAGE "\tbuffer->status=databaseStatus;\n";
    print MAKEPACKAGE "\tMakeDBPackage(buffer);\n";
    print MAKEPACKAGE "\treturn buffer;\n}\n\n";

    print MAKEPACKAGE "void ".$tablename."::MakeDBPackage(DBPackage::ServerObjectPackage *buffer) const\n{\n";
    print MAKEPACKAGE "\tDBPackage::".$tablename."Package *myBuffer=dynamic_cast<DBPackage::".$tablename."Package*>(buffer);\n";
    print MAKEPACKAGE "\tNOT_NULL(myBuffer);\n\n";
    print MAKEPACKAGE "\tmyBuffer->data.object_id=getNetworkId();\n";
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;	
	if ($typemap{$type} ne "")
	{
	    print MAKEPACKAGE "\tmyBuffer->data.".&dbIzeName($name)."=".&getterName($name)."();\n";
	}
    }
    print MAKEPACKAGE "\n\tNetworkObject::MakeDBPackage(buffer);\n}\n\n";

    ########################################################
    # Print out the LoadFromPackage functions
    ########################################################

    #constructor -- going away
#      if ($depend{$tablename} eq "")
#      {
#  	print MAKEPACKAGE $tablename."::".$tablename."(DBPackage::ServerObjectPackage *buffer) : NetworkObject(buffer)\n";
#      }
#      else
#      {
#  	print MAKEPACKAGE $tablename."::".$tablename."(DBPackage::ServerObjectPackage *buffer) : ".$depend{$tablename}."(buffer)\n";
#      }

#      print MAKEPACKAGE "{\n";
#      print MAKEPACKAGE "\tDBPackage::".$tablename."Package *myPackage=dynamic_cast<DBPackage::".$tablename."Package*>(buffer);\n";
#      print MAKEPACKAGE "\tDEBUG_FATAL(myPackage==0,(\"Attempt to create a ".$tablename." from something that isn't a ".$tablename."Package\"));\n\n";

#      foreach $colrec (split /\|/,$col)
#      {
#  	($name,$type) = split / /,$colrec;	
#  	if ($typemap{$type} ne "")
#  	{
#  	    if ($cast{$type} ne "")
#  	    {
#  		print MAKEPACKAGE "\t".&setterName($name)."(static_cast<".$type.">(myPackage->data.".&dbIzeName($name).".getValue()));\n";
#  	    }
#  	    else
#  	    {
#  		print MAKEPACKAGE "\t".&setterName($name)."(myPackage->data.".&dbIzeName($name).".getValue());\n";
#  	    }
#  	}
#      }
#      print MAKEPACKAGE "}\n\n";

    #loadFromDBPackage -- keep
    print MAKEPACKAGE "void ".$tablename."::loadFromDBPackage(DBPackage::ServerObjectPackage *buffer)\n";
    print MAKEPACKAGE "{\n"; 
    print MAKEPACKAGE "\tDBPackage::".$tablename."Package *myPackage=dynamic_cast<DBPackage::".$tablename."Package*>(buffer);\n";
    print MAKEPACKAGE "\tDEBUG_FATAL(myPackage==0,(\"Attempt to create object %i, a ".$tablename." from something that isn't a ".$tablename."Package.  May indicate that the object's type_id doesn't match its template.\",buffer->getObjectId()));\n\n";
    print MAKEPACKAGE "\tNetworkObject::loadFromDBPackage(buffer);\n\n";
    foreach $colrec (split /\|/,$col)
    {
	($name,$type) = split / /,$colrec;	
	if ($typemap{$type} ne "")
	{
	    if ($cast{$type} ne "")
	    {
		print MAKEPACKAGE "\t".&setterName($name)."(static_cast<".$type.">(myPackage->data.".&dbIzeName($name).".getValue()));\n";
	    }
	    else
	    {
		print MAKEPACKAGE "\t".&setterName($name)."(myPackage->data.".&dbIzeName($name).".getValue());\n";
	    }
	}
    }
    print MAKEPACKAGE "}\n\n";
    
    ########################################################
    # Print out the save & load package functions
    ########################################################

    print SAVEPACKAGE "void DBPackage::".$tablename."Package::finishLoad(Snapshot *snap, DB::Session *ses)\n";
    print SAVEPACKAGE "{\n";
    print SAVEPACKAGE "\tUNREF(ses);\n";
    print SAVEPACKAGE "\tUNREF(snap);\n\n";

    print SAVEPACKAGE "\tDBQuery::".$tablename."Query q;\n";
    print SAVEPACKAGE "\tq.selectMode();\n";
    print SAVEPACKAGE "\tdata.object_id=baseData.object_id;\n";
    print SAVEPACKAGE "\tq.setData(&data);\n\n";

    print SAVEPACKAGE "\tses->exec(&q);\n";
    print SAVEPACKAGE "\tif (!q.fetch())\n";
    print SAVEPACKAGE "\t{\n";
    print SAVEPACKAGE "\t\tFATAL (true,(\"Unable to retrive additional data needed by object %i\",baseData.object_id.getValue()));\n";
    print SAVEPACKAGE "\t}\n\n";
    
    print SAVEPACKAGE "\tq.done();\n";
    print SAVEPACKAGE "}\n\n";

    print SAVEPACKAGE "void DBPackage::".$tablename."Package::save(Snapshot *snap, DB::Session *ses)\n";
    print SAVEPACKAGE "{\n";
    print SAVEPACKAGE "\tDBQuery::".$tablename."Query q;\n\n";

    print SAVEPACKAGE "\tswitch(status)\n";
    print SAVEPACKAGE "\t{\n";
    print SAVEPACKAGE "\t\tcase ::NetworkObject::NEW_OBJECT:\n";
    print SAVEPACKAGE "\t\t\tq.insertMode();\n";
    print SAVEPACKAGE "\t\t\tbreak;\n\n";
    
    print SAVEPACKAGE "\t\tcase ::NetworkObject::CHANGED:\n";
    print SAVEPACKAGE "\t\t\tq.updateMode();\n";
    print SAVEPACKAGE "\t\t\tbreak;\n\n";
    
    print SAVEPACKAGE "\t\tdefault:\n";
    print SAVEPACKAGE "\t\t\tFATAL(true,(\"Unsupported database status.\"));\n";
    print SAVEPACKAGE "\t}\n\n";

    print SAVEPACKAGE "\tq.setData(&data);\n";
    print SAVEPACKAGE "\tses->exec(&q);\n";
    print SAVEPACKAGE "\tq.done();\n";

    print SAVEPACKAGE "\tServerObjectPackage::save(snap,ses);\n";
    print SAVEPACKAGE "}\n\n";

}


sub parseBPM
{
    $_ =~ /BPM\s*(\w*)/;
    $tablename=$1;
    if ($_ =~ /BPM\s*\w+\s*\:\s*(\w+)/)
    {
	$depend{$tablename} = $1;
    }
    $_=<>;
    while (!($_ =~ /\/\/.*EPM/))
    {
	if ($_ =~ /([^\s]+)\s+([^\s]+)\;/) 
	{
	    ($type,$name)=($1,$2);
	    $columns{$tablename}=$columns{$tablename}.$name." ".$type."|";
	}
	$_=<>;
    }
}

sub dbIzeName
{
    my($name)=@_;
    $name =~ s/([a-z])([A-Z])/$1_$2/g;
    $name =~ tr/[A-Z]/[a-z]/;
    return $name;
}

sub getterName
{
    my($name)=@_;
    $name = "get".ucfirst ($name);
    return $name;
}

sub setterName
{
    my($name)=@_;
    $name = "set".ucfirst ($name);
    return $name;
}
