#!/usr/bin/perl

while (<>)
{
    if (/\-\- BEGIN GENERATED CODE \(make_copy_script.pl\)/)
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
	if (/objects/)
	{
	    &makeCopyCommand($_);
	}
    }

    close (TABLIST);
}

sub makeCopyCommand
{
    my ($tablename)=@_;
    my ($col,@collist,@vallist);

    open (TABLEDEF,"../schema/${tablename}.tab");
    while (<TABLEDEF>)
    {
	if (/^\s*(\w+)/)
	{
	    $col = $1;
	    if ($col ne "create" && $col ne "primary")
	    {
		push(@collist,$col);
		if ($col eq "object_id")
		{
		    push(@vallist,"object_id + object_id_offset");
		}
		elsif ($col eq "scene_id")
		{
		    push(@vallist,"p_dest_scene");
		}
		elsif ($col eq "contained_by")
		{
		    push(@vallist,"decode(contained_by, 0,0, contained_by + object_id_offset)");
		}
		else
		{
		    push(@vallist,$col);
		}
	    }
	}
    }
    close (TABLEDEF);

    print $tabs."forall oid in objectsToCopy.FIRST..objectsToCopy.LAST\n";
    print $tabs."\tinsert into $tablename (".join(", ",@collist).")\n";
    print $tabs."\tselect ".join(", ",@vallist)."\n";
    print $tabs."\tfrom buildcluster.$tablename\n";
    print $tabs."\twhere object_id=objectsToCopy(oid);\n\n";
}
