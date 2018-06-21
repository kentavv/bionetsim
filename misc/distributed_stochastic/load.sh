#!/usr/bin/perl -w

for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == 7) {
    print "Skipping $host\n";
    next;
  }
  print "$host\t";
  print `ssh $host uptime`;
}

