#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v16 to v17..."
  cp $i $i.v16
  sed -e 's/<Format>16<\/Format>/<Format>17<\/Format>/g' \
      -e 's/<Pathway>/<Network>/g' \
      -e 's/<\/Pathway>/<\/Network>/g' \
      -e 's/<Program>/<Info><Program>/g' \
      -e 's/<\/Version>/<\/Version><\/Info>/g' \
      $i.v16 > $i
end

echo "The Pathway tag has been renamed to Network..."
echo "The Program, Format, SaveDateTime, and Version tages have been grouped into the Info tag region..."
echo "Load and save files to introduce Subnetwork tags"


