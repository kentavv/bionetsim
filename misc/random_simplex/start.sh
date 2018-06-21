#!/usr/bin/perl -w

$param_file = "triple2.param";

`mkdir -p ~/a/network_tools/random_simplex/logs`;
$dir = "~/a/network_tools/random_simplex";

for($i=1; $i<=120; $i++) {
  $host = sprintf("cn%03d", $i);
  if($i == 2 || $i == 7) {
    print "Skipping $host\n";
    next;
  }
  print "Starting random_simplex on $host\n";
  `ssh -x -f $host "(cd $dir; ./random_simplex $param_file) < /dev/null >& $dir/logs/$host\_cpu1 &" `;
  `ssh -x -f $host "(cd $dir; ./random_simplex $param_file) < /dev/null >& $dir/logs/$host\_cpu2 &" `;
}

