#!/usr/bin/perl

while (<>)
{
    if (/\-\- BEGIN GENERATED CODE \(make_fullwipe_script.pl\)/)
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
	&makeDeleteCommand($_) unless ($_ eq "version_number");
    }

    close (TABLIST);
}

sub makeDeleteCommand
{
    my ($tablename)=@_;
   
    print $tabs."delete from $tablename;\n";
}
