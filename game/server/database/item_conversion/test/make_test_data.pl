#!/usr/bin/perl

foreach $file (@ARGV)
{
    &parseFile($file);
}

print "delete cnvtest_objvar_patterns;\n";

open (RANGEFILE,"objvar_ranges.txt");
<RANGEFILE>;
while (<RANGEFILE>)
{
    chop;
    ($objvar, $min, $max) = split "\t";
    print "insert into cnvtest_objvar_patterns values ('$objvar',0,$min,$max);\n";   
}
close RANGEFILE;

foreach $objvar (keys %objvarToRemove)
{
    $objvar =~ s/\*/%/;
    print "insert into cnvtest_objvar_patterns values ('".$objvar."',1,null,null);\n";
}

foreach $script (keys %scriptToRemove)
{
    $script =~ s/\*/%/;
    print "insert into cnvtest_removed_scripts values ('".$script."');\n";
}

sub parseFile
{
    open (INFILE, $_[0]);

    $_=<INFILE>;
    @columnNames=split('\t');

    while (<INFILE>)
    {
	@data=split('\t');
	for ($i=0; $i<@data; ++$i)
	{
	    $column{$columnNames[$i]}=$data[$i];
	}
	$objvarRemove = $column{"Objvar Remove"};

	$objvarRemove =~ s/^\"//;
	$objvarRemove =~ s/\"$//;
	@objvarList=split(',',$objvarRemove);
	for ($i=0; $i<@objvarList; ++$i)
	{
	    $objvarToRemove{$objvarList[$i]}=1;
	}

	$objvarRemove = $column{"RemoveObjVar"};

	$objvarRemove =~ s/^\"//;
	$objvarRemove =~ s/\"$//;
	@objvarList=split(',',$objvarRemove);
	for ($i=0; $i<@objvarList; ++$i)
	{
	    $objvarToRemove{$objvarList[$i]}=1;
	}

	$scriptRemove = $column{"Script Remove"};

	$scriptRemove =~ s/^\"//;
	$scriptRemove =~ s/\"$//;
	@scriptList=split(',',$scriptRemove);
	for ($i=0; $i<@scriptList; ++$i)
	{
	    $scriptToRemove{$scriptList[$i]}=1;
	}

	$scriptRemove = $column{"RemoveScript"};

	$scriptRemove =~ s/^\"//;
	$scriptRemove =~ s/\"$//;
	@scriptList=split(',',$scriptRemove);
	for ($i=0; $i<@scriptList; ++$i)
	{
	    $scriptToRemove{$scriptList[$i]}=1;
	}
    }
}
