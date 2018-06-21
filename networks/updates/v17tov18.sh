#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v17 to v18..."
  cp $i $i.v17
  sed -e 's/<Format>17<\/Format>/<Format>18<\/Format>/g' \
      -e 's/<NumReplicants>/<NumReplicates>/g' \
      -e 's/<\/NumReplicants>/<\/NumReplicates>/g' \
      $i.v17 > $i
end

echo "The NumReplicants has been renamed NumReplicates..."


