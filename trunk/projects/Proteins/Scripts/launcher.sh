#!/bin/sh

function launch {
  targetName=`echo $target | sed 's/(/./g'`
  targetName=`echo $targetName | sed 's/)/./g'`
	name="${prefix}${targetName}_${nbPass}_${nbIteration}_${wAA}_${wPSSM}_${wProp}_${wSS3}_${wSS8}_${wSA}_${regularizer}_${initialLearningRate}_${numberIterationLearningRate}"

	expected_time=`expr $nbPass + 1`
	expected_time=`expr $expected_time '*' 2`
	
qsub << EOF
#$ -l h_vmem=2G 
#$ -l h_rt=${expected_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./TrainStepByStepSS ProteinsDirectory /scratch/jbecker/${database} PrefixResults ${name} CurrentFold 0 PredictionTargets "${target}" MaximumIteration ${nbIteration} AminoAcidWindow ${wAA} PSSMWindow ${wPSSM} NumberOfAAIndex ${aaindex} PropertyWindow ${wProp} SS3Window ${wSS3} SS8Window ${wSS8} SAWindow ${wSA} Regularizer ${regularizer} InitialLearningRate ${initialLearningRate} NumberIterationLearningRate ${numberIterationLearningRate} UseSavedModel ${useSavedModel} SavedModelDirectory ${savedModelDirectory} ${other}
EOF

}

target="(SS3)10"
nbIteration="5"
nbPass="10"

wAA="9"
wPSSM="11"
wSS3="10"
wSS8="10"
wSA="10"
wProp="50"
aaindex="0"

regularizer="10"
initialLearningRate="2"
numberIterationLearningRate="250000"

useSavedModel="false"
savedModelDirectory="USED_MODEL_SS3-SS8"

prefix="CHECK_ENTROPY"
database="SmallPDB/last_version"

#for target in SA SS3-SA SS8-SA SS3-SS8-SA
#for target in SS3 SS8 SA DR BBB SS3-SS8 SS3-SA SS3-DR SS3-BBB SS8-SA SS8-DR SS8-BBB SA-DR SA-BBB DR-BBB SS3-SS8-SA SS3-SS8-DR SS3-SS8-BBB SS3-SA-DR SS3-SA-BBB SS3-DR-BBB SS8-SA-DR SS8-SA-BBB SS8-DR-BBB SA-DR-BBB SS3-SS8-SA-DR SS3-SS8-SA-BBB SS3-SS8-DR-BBB SS3-SA-DR-BBB
#for i in SegCon Pos Len SegCon-Pos SegCon-Len Pos-Len SegCon-Pos-Len
#do
#	prefix="NEW_FEATURE_${i}"
#	savedModelDirectory="${prefix}_${target}"
#	other="TestFeatures ${i}"
#  launch
#done

#for drProb in 0.05 0.1 0.15 0.2 0.25 0.30 0.35 0.40 0.45 0.5 0.65 0.70 0.75 0.80 0.85 0.90 0.95 1
#do
#	prefix="AAI_${aaindex}"
#	savedModelDirectory="${prefix}_${target}"
#  prefix="DRPROB_$drProb"
#  savedModelDirectory="$prefix"
#  other="DRProbability $drProb"
	launch
#done
