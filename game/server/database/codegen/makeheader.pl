#!/usr/bin/perl

while (<>)
{
    chop;
    if (/procedure/ || /function/)
    {
	$print = 1;
    }
    if (/begin/ || /\s+as/)
    {
	if ($print)
	{
	    $print = 0;
	    print ";\n";
	}
    }
    s/\t/ /;
    s/(\s)\s+/$1/g;
    s/\(\s+/\(/g;
    s/\s+\)/\)/g;
    print if ($print)
}
