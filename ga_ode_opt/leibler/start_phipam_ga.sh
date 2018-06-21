#!/usr/bin/perl -w

$dir = "/home/f493418/network_tools/ga_ode_opt/leibler";
$ga_ode_opt = "$dir/ga_ode_opt";
$phipam = "$dir/phipam";

chdir("$dir/$ARGV[0]") || die;

$ip=`/sbin/ifconfig eth0 | grep inet | cut -d: -f2- | cut -d' ' -f1`;

print("Master ip: $ip\n");

if($ga_pid = fork()) {
    #parent
} elsif(defined $ga_pid) {
    #child
    open(STDOUT, "> log");
    open(STDERR, ">&STDOUT");
    exec("$ga_ode_opt $ARGV[1] master");
} else {
    die "cannot fork: $!\n";
}

sleep(10);

if($pp_pid = fork()) {
    #parent
} elsif(defined $pp_pid) {
    #child
    open(STDOUT, "> /dev/null");
    open(STDERR, ">&STDOUT");
    exec("$phipam -v $dir/ga_ode_opt $ARGV[1] $ip");
} else {
    die "cannot fork: $!\n";
}

waitpid($ga_pid, 0);

kill 9, $pp_pid;
