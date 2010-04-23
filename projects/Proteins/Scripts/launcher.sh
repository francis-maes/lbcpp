#!/bin/sh

function launch {
	name="${prefix}_${target}_${nbPass}_${nbIteration}_${wAA}_${wPSSM}_${wProp}_${wSS3}_${wSS8}_${wSA}_${regularizer}_${initialLearningRate}_${numberIterationLearningRate}"

	expected_time=`expr $nbPass + 1`
	#expected_time=`expr $expected_time '*' 3`
	
qsub << EOF
#$ -l h_vmem=2G 
#$ -l h_rt=${expected_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./TrainStepByStepSS ProteinsDirectory /scratch/jbecker/CB513 PrefixResults ${name} CurrentFold 0 PredictionTarget ${target} MaximumIteration ${nbIteration} MaximumPass ${nbPass} AminoAcidWindow ${wAA} PSSMWindow ${wPSSM} PropertyWindow ${wProp} SS3Window ${wSS3} SS8Window ${wSS8} SAWindow ${wSA} Regularizer ${regularizer} InitialLearningRate ${initialLearningRate} NumberIterationLearningRate ${numberIterationLearningRate} UseSavedModel ${useSavedModel} SavedModelDirectory ${savedModelDirectory}
EOF

}

target="SS3-SS8"
nbIteration="5"
nbPass="10"

wAA="9"
wPSSM="11"
wSS3="10"
wSS8="10"
wSA="10"
wProp="0"

regularizer="10"
initialLearningRate="2"
numberIterationLearningRate="250000"

useSavedModel="false"
savedModelDirectory="MODEL_SS3-SS8_10_5_9_11_10_10_10_10_2_250000/decorated.inference"

prefix="TRUEAA"


#for target in SS3 SS8 SA SS3-SS8 SS3-SA SS8-SA SS3-SS8-SA
#do
#	launch
#done

for wProp in 2 5 10 20 50 80 100 150
do
	launch
done
