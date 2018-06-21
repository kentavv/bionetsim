#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v8 to v9..."
  cp $i $i.v8
  sed -e 's/<Format>8<\/Format>/<Format>9<\/Format>/g' \
      $i.v8 > $i
end

echo "You will need to load and save each file to initialize the PhaseShift tag..."

