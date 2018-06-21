#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v12 to v13..."
  cp $i $i.v12
  sed -e 's/<Format>12<\/Format>/<Format>13<\/Format>/g' \
      $i.v12 | grep -v "CanvasSize" > $i
end

echo "The CanvasSize tag has been removed..."


