#!/usr/bin/perl

while (<>)
{
    if (/\-\- BEGIN GENERATED CODE \(make_purge_script.pl\)/)
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
	if (/objects/ && ($_ ne "objects"))
	{
	    &makeCommand($_);
	}
    }

    close (TABLIST);
}

sub makeCommand
{
    my ($tablename)=@_;

    print $tabs."delete from ${tablename} where not exists (select * from objects where ${tablename}.object_id = objects.object_id);\n";
}
