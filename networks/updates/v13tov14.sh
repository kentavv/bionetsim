#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v13 to v14..."
  cp $i $i.v13
  sed -e 's/<Format>13<\/Format>/<Format>14<\/Format>/g' \
      $i.v13 | grep -v "CurrentSubNetwork" | grep -v "UpdateSimulationAutomatically" > $i
end

echo "The CurrentSubNetwork tag has been removed..."
echo "Disabling UpdateSimulationAutomatically by default..."


