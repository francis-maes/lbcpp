#!/bin/sh

function launch {
	name="${prefix}_${target}_${nbPass}_${nbIteration}_${wAA}_${wPSSM}_${wSS3}_${wSS8}_${wSA}_${regularizer}"

	expected_time=`expr $nbPass + 1`
	#expected_time=`expr $expected_time '*' 3`
	
qsub << EOF
#$ -l h_vmem=2G 
#$ -l h_rt=${expected_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./TrainStepByStepSS ProteinsDirectory /scratch/jbecker/CB513 PrefixResults ${name} CurrentFold 0 PredictionTarget ${target} MaximumIteration ${nbIteration} MaximumPass ${nbPass} AminoAcidWindow ${wAA} PSSMWindow ${wPSSM} SS3Window ${wSS3} SS8Window ${wSS8} SAWindow ${wSA} Regularizer ${regularizer}
EOF

}

target="SS3"
nbIteration="15"
nbPass="3"

wAA="9"
wPSSM="11"
wSS3="10"
wSS8="10"
wSA="10"

regularizer="10"
prefix="CHECK"


#for target in SS3 SS8 SA SS3-SS8 SS3-SA SS8-SA SS3-SS8-SA
#do
	launch
#done
