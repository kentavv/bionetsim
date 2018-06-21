foreach i (n1 n1a n2 n2a n3 n3a n4 n4a)
 cd $i
 foreach j (000 001 002)
  ~/network_tools/time_series/time_series $j/best.net > a
  ~/network_tools/time_series/time_series $j/$i.net > b
  gnuplot ../$i.gp 
  mv comp.png comp_$j.png
 end
 cd ..
end
