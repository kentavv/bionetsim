#!/bin/tcsh -f

foreach i (`find . -name \*.net`)
  echo "Converting $i from v7 to v8..."
  cp $i $i.v7
  sed -e 's/<Format>7<\/Format>/<Format>8<\/Format>/g' \
      -e 's/<FitnessFunction>0<\/FitnessFunction>/<FitnessFunction>Null<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>1<\/FitnessFunction>/<FitnessFunction>Glu6p Presence<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>2<\/FitnessFunction>/<FitnessFunction>Experimental Match<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>3<\/FitnessFunction>/<FitnessFunction>Constant Glucose<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>4<\/FitnessFunction>/<FitnessFunction>Glu6p * Constant Glucose<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>5<\/FitnessFunction>/<FitnessFunction>Glu6p Entropy<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>6<\/FitnessFunction>/<FitnessFunction>Flux \/ Cost<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>7<\/FitnessFunction>/<FitnessFunction>Glu6p * Constant Glucose * Flux<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>8<\/FitnessFunction>/<FitnessFunction>Z Steady State<\/FitnessFunction>/g' \
      -e 's/<FitnessFunction>9<\/FitnessFunction>/<FitnessFunction>R Rate<\/FitnessFunction>/g' \
      $i.v7 > $i
end

echo "You will need to load and save each file to initialize the SimulationMethod tag..."

