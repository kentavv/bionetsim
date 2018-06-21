#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v9 to v10..."
  cp $i $i.v9
  sed -e 's/<Format>9<\/Format>/<Format>10<\/Format>/g' \
      $i.v9 > $i
end

echo "It is now possible to disable plotting of individual concentrations."

