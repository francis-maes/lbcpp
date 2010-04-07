#!/bin/sh

function launch {

	name=`echo "$prefix.$nbFolds $nbIter $learningRate $model" | sed 's/ /_/g'`

	for(( targetFold=0; targetFold < $nbFolds; targetFold++ ))
	do
		qsub << EOF
#$ -l h_vmem=16G 
#$ -l h_rt=24:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N $name.${targetFold}
./SecondaryStructureCrossValidation ~/CASP9/CB513 $name.${targetFold} $nbFolds ${targetFold} $nbIter $regularizer $learningRate $model
EOF
	done

}


nbFolds="7"
nbIter="100"
learningRate="Inv 2 250000"
model="CO"
prefix="SS"
regularizer=0.0001

for i in 100000 150000 500000
do
	learningRate="Inv 2 $i"
	launch
done
