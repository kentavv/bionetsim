#!/usr/bin/perl -w

for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == -1) {
    print "Skipping $host\n";
    next;
  }
  print "Shutting comm_test down on $host\n";
  `ssh $host killall comm_test`;
}

