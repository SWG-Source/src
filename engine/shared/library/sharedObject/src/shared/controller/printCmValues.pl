$value = 0;

while (<>)
{
	if (/(CM_[^,]+)/)
	{
		print "$1: $value\n";
		++$value;
	}
}