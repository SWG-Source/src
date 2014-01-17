print "delete object_templates;\n";

while (<>)
{
    chop;
    ($id,$template)=split("\t");
    print "insert into object_templates values (".unpack("i",pack("I",oct($id))).",'$template');\n";
}

print "exit;\n";

