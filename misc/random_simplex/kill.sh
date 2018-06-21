#!/usr/bin/perl -w

for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == 2 || $i == 7) {
    print "Skipping $host\n";
    next;
  }
  print "Shutting random_simplex down on $host\n";
  `ssh $host killall random_simplex`;
}

