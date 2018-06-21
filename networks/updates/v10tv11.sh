#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v10 to v11..."
  cp $i $i.v10
  sed -e 's/<Format>10<\/Format>/<Format>11<\/Format>/g' \
      $i.v10 > $i
end

echo "Only the Format tag has been updated..."
echo "File format v11 introduces Ids which would be too complicated to implement here"

