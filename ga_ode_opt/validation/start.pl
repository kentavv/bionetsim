#!/usr/bin/perl -w

foreach $i ("n1", "n1a", "n2", "n2a", "n3", "n3a", "n4", "n4a") {
  for($j=0; $j<3; $j++) {
    $dir = sprintf("$i/%03d", $j);
    `mkdir -p $dir`;
    chdir($dir);
    `ln -sf ../../$i.net ../../$i.param .`;
    $cmd = sprintf("bsub '../../../ga_ode_opt $i.param > $i.out'");
    `$cmd`;
    chdir("../..");
  }
  print "$i\n";
}

