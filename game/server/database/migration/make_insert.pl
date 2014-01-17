#!/usr/bin/perl

# Given an export from Postgresql, convert it into a series of insert statements
# that Oracle can understand.
# Assumes the data is stored in files name <tablename>.txt

# This is suitable for small databases, but should not be used for large databases
# because it inserts the data one row at a time.

foreach $filename (@ARGV)
{
    print STDERR $filename."\n";

    open (INFILE,$filename);
    $tablename=$filename;
    $tablename =~ s/\.txt//;

    while (<INFILE>)
    {
	chop;
	$_="'".$_."'";
	s/\t/','/g;
	s/\'\\N\'/NULL/g;
	print "insert into $tablename values ($_);\n";
	if ((++$linecount)== 1000)
	{
	    print "commit;\n";
	    $linecount = 0;
	}
    }
    close (INFILE);
}

print "commit;\n";
