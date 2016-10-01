#!/usr/bin/awk -f

BEGIN { count = 0}
1{
		if( NR != 1) 
		{ 
			array_s[$3]++
			array_t[$3] = array_t[$3] + $7
		}
	 }
END { 
	print "size   ref   time_avg(ns)"    

	for(i = 0 ; i <length(array_s);++i)
	{
		if(array_s[i] !=  0)
		{
			print i"  " array_s[i]  "  " array_t[i]/array_s[i]; 
		}
	}
}



