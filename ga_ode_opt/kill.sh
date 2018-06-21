#!/usr/bin/perl -w

lsgrun -v -n 120 killall ga_ode_opt
 
exit
 
for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == 7) {
    print "Skipping $host\n";
    next;
  }
  print "Shutting ga_ode_opt down on $host\n";
  `ssh $host killall ga_ode_opt`;
}

