#!/usr/bin/perl

$parent="";

while (<>)
{
    chop;
    @data=split("\t");
    $id=shift (@data);
    $shortname=shift (@data);
    
    $fullname="";
    $found = 0;
    for ($i=0; $i<8; ++$i)
    {
	if ($found)
	{
	    $previousname[$i]="";
	}
	else
	{
	    if ($data[$i] eq "")
	    {
		$data[$i]=$previousname[$i];
	    }
	    else
	    {
		$found = 1;
		$classname=$data[$i];
		$previousname[$i]=$data[$i];
	    }
	    $fullname.=".".$data[$i];
	}
    }
    $class_id{$fullname}=$id;
    $parentname=$fullname;
    $parentname =~ s/\.[^\.]+$//;
    $parent_id=$class_id{$parentname} + 0;

#    print join("\t",@data)."\n";
    $max_types     = $data[8] + 0;
    $min_types     = $data[9] + 0;
    $min_pools     = $data[10] + 0;
    $max_pools     = $data[11] + 0;
#    $min_pool_size = $data[11] + 0;
#    $max_pool_size = $data[12] + 0;

#    $fractal_type       = $data[13] + 0;
#    $fractal_x_scale    = $data[14] + 0;
#    $fractal_y_scale    = $data[15] + 0;
#    $fractal_bias       = $data[16] + 0;
#    $fractal_gain       = $data[17] + 0;	
#    $fractal_combo_rule = $data[18] + 0;
#    $fractal_frequency  = $data[19] + 0;
#    $fractal_amplitude  = $data[20] + 0;
#    $fractal_octaves    = $data[21] + 0;

    for ($i=0; $i!=11; ++$i)
    {
	$attribute_name[$i] = $data[$i+12];
	$attribute_min[$i]  = $data[$i*2+23];
	$attribute_max[$i]  = $data[$i*2+24];
    }

    $resource_container_type = $data[45];
    $random_name_class = $data[46];

#### output:

    $classname =~ s/'/''/g;
    print "insert into resource_import values ($id,$parent_id,'$shortname','$classname',$max_types,$min_types,$min_pools,$max_pools,'$resource_container_type','$random_name_class');\n";
    
    for ($i=0; $i!=11; ++$i)
    {
	last unless ($attribute_name[$i] ne "");
	print "insert into res_imp_object_variables values ($id,'attributes.$attribute_name[$i]',1,'$attribute_min[$i]:$attribute_max[$i]:');\n"
    }
}
