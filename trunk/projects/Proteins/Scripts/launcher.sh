#!/bin/sh

function launch {
  targetName=`echo $target | sed 's/(/./g'`
  targetName=`echo $targetName | sed 's/)/./g'`
	name="${prefix}${targetName}"

	expected_time=`expr $nbPass + 1`
	expected_time=`expr $expected_time '*' 3`
	
qsub << EOF
#$ -l h_vmem=2G 
#$ -l h_rt=30:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./${program} ProteinsDirectory /scratch/jbecker/${database} PrefixResults ${name} CurrentFold 0 PredictionTargets "${target}" MaximumIteration ${nbIteration} AminoAcidWindow ${wAA} PSSMWindow ${wPSSM} NumberOfAAIndex ${aaindex} PropertyWindow ${wProp} SS3Window ${wSS3} SS8Window ${wSS8} SAWindow ${wSA} Regularizer ${regularizer} InitialLearningRate ${initialLearningRate} NumberIterationLearningRate ${numberIterationLearningRate} UseSavedModel ${useSavedModel} SavedModelDirectory ${savedModelDirectory} ${other}
EOF

}

program="TrainStepByStepSS"

target="(DR)10"
nbIteration="5"

wAA="9"
wPSSM="11"
wSS3="10"
wSS8="10"
wSA="10"
wProp="0"
aaindex="0"

regularizer="10"
initialLearningRate="2"
numberIterationLearningRate="250000"

useSavedModel="true"
savedModelDirectory="TEST_MODEL_DR"

prefix="MODEL_DR"
database="PDB70/protein"

other="DRProbability 0.95 TestFeatures Pos"

#for target in `python CombinaisonGenerator.py StAl BBB DR SA SS8 SS3`

#-------------------- Learning Rate ---------------------#
numberIterations=""
for exposant in {3..6}
do
  value=`echo "10 ^ ${exposant}" | bc`
  for multi in 1 2 5
  do
    iteration=`echo "${value} * ${multi}" | bc`
    numberIterations="${numberIterations} ${iteration}"
  done
done

echo "Number Iterations: ${numberIterations}"

initialLearningRates=""
for rates in 0.0001 0.001 0.01 0.1 1 10 100 1000 10000
do
  for multi in 1 2 5
  do
    learningRate=`echo "${rates} * ${multi}" | bc`
    initialLearningRates="${initialLearningRates} ${learningRate}"
  done
done

echo "Initial Learning Rate: ${initialLearningRates}"

for numberIteration in $numberIterations
do
  for learningRate in $initialLearningRates
  do
    for target in SS3 SS8 SA DR BBB StAl
    do
      program="LEARNING_RATE"
      prefix="LR_MP_${learningRate}_${numberIteration}"
      savedModelDirectory="${prefix}_${target}"
      target="($target)10"

      initialLearningRate=${learningRate}
      numberIterationLearningRate=${numberIteration}

      launch
    done
  done
done
#------------------- END Learning Rate ------------------#


#---------------------- Regularizer ---------------------#
#reg_list=""
#for reg in 0.0001 0.001 0.01 0.1 1 10 100 1000
#do
#for multi in 1 2 5
#do
#  regularizer=`echo "${reg} * ${multi}" | bc`
#  reg_list="$reg_list $regularizer"
#done
#done
#
#echo "REGs: " $reg_list
#
#for regularizer in 0 $reg_list 3000 4000 6000 10000
#do
#for target in SS3 SS8 SA DR BBB StAl
#do
#  program="REGULARIZER"
#	prefix="REG_MP_${regularizer}"
#  savedModelDirectory="REG_MP_${target}_${regularizer}"
#  target="($target)10"
#  launch
#done
#done
#-------------------- END Regularizer --------------------#

#for wStAl in 1 2 5 10 20 30 40 50 60 70 80 90 100 110
#do
#for wStAlFreq in 1 2 5 10 20 30 40 50 60 70 80 90 100 110
#do
#  program="ref"
#  savedModelDirectory="ref_${wStAl}_${wStAlFreq}"
#  prefix=$savedModelDirectory
#  target="(${targ})10"
#  other="DRProbability 0.95 TestFeatures Pos DRRegularizer 10 SARegularizer 500 StAlWindow ${wStAl} StAlWindowFreq ${wStAlFreq}"
#  launch
#done
#done
