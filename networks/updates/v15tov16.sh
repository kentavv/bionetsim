#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v15 to v16..."
  cp $i $i.v15
  sed -e 's/<Format>15<\/Format>/<Format>16<\/Format>/g' \
      $i.v15 | grep -v "SimulationDelay" > $i
end

echo "The SimulationDelay tag has been removed..."


