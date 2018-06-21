#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v11 to v12..."
  cp $i $i.v11
  sed -e 's/<Format>11<\/Format>/<Format>12<\/Format>/g' \
      $i.v11 > $i
end

echo "Only the Format tag has been updated..."
echo "Load and save each file to introduce the new stochastic parameters"

