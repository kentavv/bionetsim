#!/usr/bin/perl -w

lsgrun -v -n 120 killall distributed_stochastic

exit

for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == -1) {
    print "Skipping $host\n";
    next;
  }
  print "Shutting distributed_stochastic down on $host\n";
  `ssh $host killall distributed_stochastic`;
}

