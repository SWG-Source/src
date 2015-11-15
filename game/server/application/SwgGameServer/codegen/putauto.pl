#!/usr/bin/perl

@files=@ARGV;

foreach $file (@files)
{
    print "Working on $file\n";
    
    open (INFILE,$file);
    open (OUTFILE,">".$file.".tmp");
    while (<INFILE>)
    {
	print OUTFILE $_;
	if ((/\/\/\s*From\s+([^\s]+)/) ||
	    (/--\s*From\s+([^\s]+)/))
	{
	    $from=$1;
	}
	if ((/\/\/ BEGIN AUTO_GENERATED CODE\s*\#/) ||
	    (/-- BEGIN AUTO_GENERATED CODE/))
	{
	    &insertFile($from);
	    while (!(/\/\/ END AUTO_GENERATED CODE\s*\#/) &&
		   !(/-- END AUTO_GENERATED CODE/))
	    {
		$_=<INFILE>;
	    }
	    print OUTFILE $_;
	}
    }
    close (OUTFILE);
    close(INFILE);

    $rc=system("diff --brief $file.tmp $file");
    if ($rc!=0)
    {
	system ("cp $file.tmp $file");
    }
    system("rm $file.tmp");
}

sub insertFile
{
    print OUTFILE "\n";

    my($filename)=@_;
    open (INSFILE,$filename);
    while (<INSFILE>)
    {
	print OUTFILE $_;
    }
    close INSFILE;
}

