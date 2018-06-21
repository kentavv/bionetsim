#!/usr/bin/perl -w

$param_file = "opt.param";

$dir = "~/network_tools/distributed_stochastic";
`mkdir -p $dir/logs`;

$exec = "~/network_tools/distributed_stochastic/distributed_stochastic";

for($i=1; $i<=120; $i++) {
#for($i=120; $i>=1; $i--) {
  $host = sprintf("cn%03d", $i);
  if($i == -1) {
    print "Skipping $host\n";
    next;
  }
  print "Starting distributed_stochastic on $host\n";
  `ssh -x -f $host "(cd $dir; $exec $param_file) < /dev/null >& $dir/logs/$host\_cpu1 &" `;
  `ssh -x -f $host "(cd $dir; $exec $param_file) < /dev/null >& $dir/logs/$host\_cpu2 &" `;
}

