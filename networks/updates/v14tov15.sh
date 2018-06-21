#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v13 to v14..."
  cp $i $i.v13
  sed -e 's/<Format>14<\/Format>/<Format>15<\/Format>/g' \
      -e 's/AnnotationItem>/Annotation>/g' \
      $i.v13 > $i
end

echo "The AnnotationItem tag has been renamed Annotation..."


