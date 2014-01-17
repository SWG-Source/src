#!/usr/bin/perl

while (<>)
{
    if (/\-\- BEGIN GENERATED CODE \(make_fullcopy_script.pl\)/)
    {
	print;
	&generateCode;
	do
	{
	    last unless ($_=<>);
	} until (/\-\- END GENERATED CODE/)
    }
    print;
}

sub generateCode
{
    open (TABLIST,"ls -b ../schema/*.tab|");

    $tabs="\t\t";

    while (<TABLIST>)
    {
	chop;
	s/\.tab//;
	s/\.\.\/schema\///;
	&makeCopyCommand($_) unless ($_ eq "version_number");
    }

    close (TABLIST);
}

sub makeCopyCommand
{
    my ($tablename)=@_;
   
    print $tabs."insert into $tablename select * from buildcluster.$tablename;\n";
}
