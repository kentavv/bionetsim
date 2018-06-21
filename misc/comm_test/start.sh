#!/usr/bin/perl -w

$dir = "~/comm_test";
`mkdir -p $dir/logs`;

$exec = "~/comm_test/comm_test";

for($i=1; $i<=120; $i++) {
#for($i=120; $i>=1; $i--) {
  $host = sprintf("cn%03d", $i);
  if($i == -1) {
    print "Skipping $host\n";
    next;
  }
  print "Starting comm_test on $host\n";
  `ssh -x -f $host "(cd $dir; $exec) < /dev/null >& $dir/logs/$host\_cpu1 &" `;
  `ssh -x -f $host "(cd $dir; $exec) < /dev/null >& $dir/logs/$host\_cpu2 &" `;
}

