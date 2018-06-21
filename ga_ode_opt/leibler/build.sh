#!/bin/tcsh -f

set i=c242

grep Best: $i.out | cut -f2,3 > a
grep Avg: $i.out | cut -f3 | grep -v Min: > b
grep Worst: $i.out | cut -f3 > c
paste a b c > /usr/local/cluster_monitoring/apache/htdocs/ftp_not_working/$i.txt

mv best_* /usr/local/cluster_monitoring/apache/htdocs/ftp_not_working

