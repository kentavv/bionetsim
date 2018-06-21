#!/usr/bin/perl -w

$param_file = "d012_fixed.param";

$dir = "~/network_tools/ga_ode_opt/leibler";
`mkdir -p $dir/logs`;

$exec = "~/network_tools/ga_ode_opt/ga_ode_opt";

#for($i=3; $i<=120; $i++) {
for($i=120; $i>=3; $i--) {
  $host = sprintf("cn%03d", $i);
  if($i == -1) {
    print "Skipping $host\n";
    next;
  }
  print "Starting ga_ode_opt on $host\n";
  `ssh -x -f $host "(cd $dir; $exec $param_file) < /dev/null >& $dir/logs/$host\_cpu1 &" `;
  `ssh -x -f $host "(cd $dir; $exec $param_file) < /dev/null >& $dir/logs/$host\_cpu2 &" `;
}

