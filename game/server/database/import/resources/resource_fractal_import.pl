#!/usr/bin/perl

$parent="";
   
$fractalParamName[0]="type";
$fractalParamName[1]="x";
$fractalParamName[2]="y";
$fractalParamName[3]="bias";
$fractalParamName[4]="gain";
$fractalParamName[5]="combinationRule";
$fractalParamName[6]="frequency";
$fractalParamName[7]="amplitude";
$fractalParamName[8]="octaves";

while (<>)
{
    chop;

    @data=split("\t");
    $id=shift (@data);
    
    $planet        = $data[0];
    $min_pool_size = $data[1] + 0;
    $max_pool_size = $data[2] + 0;

    $planet =~ tr/A-Z/a-z/;

#### output:

    print "insert into res_imp_object_variables values ($id,'planets.$planet.minpoolsize',0,'".($min_pool_size)."');\n";
    print "insert into res_imp_object_variables values ($id,'planets.$planet.maxpoolsize',0,'".($max_pool_size)."');\n";

    for ($i=0; $i!=9; ++$i)
    {
	if ($data[$i+3] ne "")
	{
	    print "insert into res_imp_object_variables values ($id,'planets.$planet.fractal.$fractalParamName[$i]',0,'".($data[$i+3])."');\n";
	}
    }
}
