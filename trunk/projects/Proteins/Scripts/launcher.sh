#!/bin/sh

res_memory="6"
res_time="5"
res_core="1"

function launch {
  sleep 2

  echo "Job - " ${prefix}

#$ -pe snode ${res_core}
qsub << EOF
#$ -l h_vmem=${res_memory}G
#$ -l h_rt=${res_time}:00:00

#$ -m eas
#$ -M J.Becker@ULg.ac.be

#$ -cwd

#$ -N ${prefix}
./${program} ProteinsDirectory /scratch/jbecker/${database} Output ${prefix} Targets "${target}" ${other}
EOF

}

program="MoonBox"
other_=""
prefix_="e"

function DB_PSIPRED
{
  database="PSIPRED/train/"
  other_="${other_} TestingProteinsDirectory /scratch/jbecker/PSIPRED/test/"
  db_name="PSIPRED"
}

function DB_PDB30Large
{
  database="PDB30Large/xml/"
  other_=""
  db_name="PDB30Large"
}

function DB_PDB30Medium
{
  database="PDB30Medium/xml/"
  other_=""
  db_name="PDB30Medium"
}

function SingleTask
{
  res_time="150"
  res_memory="6"
  for targ in SS3
  do
    res_core="3"
    target="(${targ})10"
    DB_PSIPRED
    other="${other_} StoppingIteration 20"
    prefix="${prefix_}${db_name}.SingleTask.${targ}"
    #launch

    DB_PDB30Medium
    other="SaveIterations StoppingIteration 15 NumThreads 3 LearningRate 1 LearningStep 200000 Regularizer 0.001 ForceUse"
    prefix="${prefix_}${db_name}.Test.r1000.ParallelWithInitialization"
    launch
  done
}

function MultiTask
{
  res_time="250"
  res_memory="8"
  res_core="27"
  target="(SS3-SS8-SA-DR-StAl)10"
  other="${other_} StoppingIteration 20"
  
  DB_PDB30Medium
  lr="1"
  ls="200000"
  other="StoppingIteration 15 NumThreads ${res_core} LearningRate ${lr} LearningStep ${ls} Regularizer 0.001 ForceUse"
  prefix="${prefix_}${db_name}.969.LS${ls}.LR${lr}.REGe3"
  launch
  
  DB_PSIPRED
  other="${other_} StoppingIteration 20 NumThreads ${res_core}"
  prefix="${prefix_}${db_name}.969"
  #launch
}


#------------------------------------------------

SingleTask
#MultiTask
exit

res_time="40"
res_memory="6"

other_="StoppingIteration 20"
for targ in SS3 SA DR SS8 StAl
do
  for ls in 10000 50000 100000 500000 1000000 5000000
  do
    for lr in 0.01 0.05 0.1 0.5 1 5 10
    do
      for reg in 0
      do
        DB_PSIPRED
        #DB_PDB30
        target="(SS3-SS8-SA-DR-StAl)1(${targ})1"
        prefix="${prefix_}${db_name}.OnePass.${targ}.LS${ls}.LR${lr}.REG${reg}"
        other="${other_} ForceUse LearningStep ${ls} LearningRate ${lr} Regularizer ${reg}"
        launch
      done
    done
  done
done

exit

for lr in 2 5 10 20 50 100; do
  other="${other_} LearningRate ${lr}"
  prefix="${prefix_}_${lr}"
  launch
done
exit

for size in 3 5 7 9 11 13 15 17 21 31 41 51 71 101 201; do
  other="${other_} WindowSize ${size}"
  prefix="${prefix_}_ws_${size}"
  launch
done

exit
for targ in `python CombinaisonGenerator.py SS3 SS8 SA DR StAl`; do
  target="(${targ})5"
  launch
done

exit
for lr in 1 2 5 10 20 50; do
  other="${other_} LearningRate ${lr}"
  prefix="${prefix_}.lr${lr}"
  launch
done

exit
for ls in 1 10 100 1000 10000 100000 1000000; do
  other="${other_} LearningStep ${ls}"
  prefix="${prefix_}.ls${ls}"
  launch
done



exit

for targ in SS3 SS8 SS3-SS8 DR SA SS3-SA SS3-DR
do
  target="($targ)10"
  launch
done


exit

#for targ in `python CombinaisonGenerator.py StAl BBB DR SA SS8 SS3`
#for lr in 0.0001 0.0002 0.0005 0.001 0.002 0.005 0.01 0.02 0.05 0.1 0.2 0.5 1 2 5 10 20 50 100 200 500 1000 2000 5000 10000
#for reg in 0.000001 0.000002 0.000005 0.0000001 0.0000002 0.0000005 0.00000001 0.00000002 0.00000005 
#for ls in 100 200 500 1000 2000 5000 10000 20000 50000 1000000 2000000 5000000 10000000 20000000 50000000

#program="SimpleFeatures"
#database="PredictedDB/supervision400"
#target="(SS3)2"
#other="TestingProteinsDirectory /scratch/jbecker/PredictedDB/supervision100 SaveInference"
#nbProt=400
#launch

#exit

#nbProt="500"
#target="(SS3)10"
#other="$other SaveInference IsExperimentalMode"
#
#prefix="exp2OldPSSM"
#database="PredictedDB/supervision500/"
#launch
#
#prefix="exp2NewNormPSSM"
#database="PredictedDB/newSupervision500/"
#launch
#
#exit

prefix="exp_CM_201-1000_MP"
#database="L50DB/protein"
database="L201-1000DB"
target="(CM)10"
nbProt="1000"
other="IsExperimentalMode SaveInference StoppingIteration 10"
launch

exit

#for multitask in `python CombinaisonGenerator.py VerticalBy2 OblicBy2 HorizontalBy2` -
for multitask in OblicBy2 -
do
for targ in SS3
do
prefix="exp_MultiSimple-SS3_${multitask}"
database="PredictedDB/protein/SS3-SS8-SA-DR-BBB"
nbProt="500"
target="(${targ})1"
other="MultiTaskFeatures $multitask"
other="$other SaveInference IsExperimentalMode"
launch
done
done

