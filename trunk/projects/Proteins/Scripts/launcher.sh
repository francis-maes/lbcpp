#!/bin/sh

function launch {
  targetName=`echo $target | sed 's/(/./g'`
  targetName=`echo $targetName | sed 's/)/./g'`
	name="${prefix}${targetName}"

  nbTarget=`echo $targetName | grep -o "-" | wc -l`
	expected_time=`expr ${nbTarget} + 3`
	expected_time=`expr $expected_time '*' 10`
  echo "Expected Time: ${expected_time}"

  sleep 2

qsub << EOF
#$ -l h_vmem=1G
#$ -l h_rt=${expected_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${name}
./${program} ProteinsDirectory /scratch/jbecker/${database} Output ${name} Targets "${target}" NumProteinsToLoad 140 ${other}
EOF

}

program="SunBox"

target="(DR)10"

prefix="MODEL_DR"
database="PDB70/protein"

#other="DRProbability 0.95 TestFeatures Pos"

for targ in `python CombinaisonGenerator.py StAl BBB DR SA SS8 SS3`
#for lr in 0.0001 0.0002 0.0005 0.001 0.002 0.005 0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100 200 500 1000 2000 5000 10000
#for ls in 100 200 500 1000 2000 5000 10000 20000 50000 1000000 2000000 5000000 10000000 20000000 50000000
do
program="TunedSunBox"
prefix="TunedSunBox"
other="SaveInference"
target="(${targ})10"
launch
done

#for targ in SS8 BBB 
#do
#  target="(${targ})10"
#  prefix="RelatedTarget"
#  launch
#done

#-------------------- Learning Rate ---------------------#
#numberIterations=""
#for exposant in {3..6}
#do
#  value=`echo "10 ^ ${exposant}" | bc`
#  for multi in 1 2 5
#  do
#    iteration=`echo "${value} * ${multi}" | bc`
#    numberIterations="${numberIterations} ${iteration}"
#  done
#done
#
#echo "Number Iterations: ${numberIterations}"
#
#initialLearningRates=""
#for rates in 0.0001 0.001 0.01 0.1 1 10 100 1000 10000
#do
#  for multi in 1 2 5
#  do
#    learningRate=`echo "${rates} * ${multi}" | bc`
#    initialLearningRates="${initialLearningRates} ${learningRate}"
#  done
#done
#
#echo "Initial Learning Rate: ${initialLearningRates}"
#
#for numberIteration in $numberIterations
#do
#  for learningRate in $initialLearningRates
#  do
#    for target in SS3 SS8 SA DR BBB StAl
#    do
#      program="LEARNING_RATE"
#      prefix="LR_MP_${learningRate}_${numberIteration}"
#      savedModelDirectory="${prefix}_${target}"
#      target="($target)10"
#
#      initialLearningRate=${learningRate}
#      numberIterationLearningRate=${numberIteration}
#
#      launch
#    done
#  done
#done
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

