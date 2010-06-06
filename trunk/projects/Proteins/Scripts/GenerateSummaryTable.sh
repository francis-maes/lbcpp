#!/bin/bash

#----- CONSTANTS -------------#
TARGETS="SS3 SS8 SA DR BBB StAl"
_MINUS_INF=-10000;

#----- GLOBAL VARIABLES -----#

#----- FUNCTIONS ------------#
function OrderTarget
{
  toConvert=`echo ${1} | sed "s/SS3/1/g;s/SS8/2/g;s/SA/3/g;s/DR/4/g;s/BBB/5/g;s/StAl/6/g" |  tr '-' '\n' | sort -u`
  toConvert=`echo -${toConvert} | sed "s/ /--/g;s/-1/SS3/g;s/-2/SS8/g;s/-3/SA/g;s/-4/DR/g;s/-5/BBB/g;s/-6/StAl/g"`
  echo $toConvert
}

function IndexOfTarget
{
  case $1 in
    "SS3" )  echo "1" ;;
    "SS8")   echo "2" ;;
    "SA" )   echo "3" ;;
    "DR" )   echo "4" ;;
    "BBB" )  echo "5" ;;
    "StAl" ) echo "6" ;;
  esac
}

function FullTargetName
{
  case $1 in
    "SS3" )  echo "SecondaryStructureSequence" ;;
    "SS8")   echo "DSSPSecondaryStructureSequence" ;;
    "SA" )   echo "SolventAccessibilityThreshold20" ;;
    "DR" )   echo "DisorderProbabilitySequence" ;;
    "BBB" )  echo "BackboneBondSequence" ;;
    "StAl" ) echo "StructuralAlphabetSequence" ;;
  esac
}

function CompareToPreviousScore
{
  local currentScore=`getCurrentScore $1`
  local previousScore=`getPreviousScore $1`
  if [[ `echo "if (${currentScore} < ${2}) 1 else 0" | bc` -eq 1 ]]
  then
    setCurrentScore $1 $2
  fi

  if [[ $(echo "if (${previousScore} < ${2}) 1 else 0" | bc) -eq 1 ]]
  then
    echo "1"
  fi
}

function InitScores
{
  for task in $TARGETS
  do
    echo $_MINUS_INF > .currentScore.${task}
    echo $_MINUS_INF > .previousScore.${task}
  done
}

function FromCurrentToPreviousScores
{
  for task in $TARGETS
  do
    current=`cat .currentScore.${task}`
    previous=`cat .previousScore.${task}`

    if [[ $(echo "if (${previous} < ${current}) 1 else 0" | bc) -eq 1 ]]
    then
      mv .currentScore.$task .previousScore.$task
    fi
    echo $_MINUS_INF > .currentScore.$task
  done
}

function setCurrentScore
{
  echo $2 > .currentScore.$1
}

function getCurrentScore
{
  cat .currentScore.$1
}

function getPreviousScore
{
  cat .previousScore.$1
}

function MultiTask
{
  # Get all combinaison of order $i
  combinaisons=""
  for combi in `python CombinaisonGenerator.py ${TARGETS}`
  do
    if [ `echo ${combi} | grep -o '-' | wc -l` -eq $1 ]
    then
      combinaisons="${combinaisons} ${combi}"
    fi
  done
  # Generate scores for each combinaison associated to another task
  for combi in $combinaisons
  do
    echo -n "  Multi-Task +${combi}"
  
    for task in $TARGETS
    do
      # Scores of order less than $i
      value=""
      if [ `echo ${combi} | grep ${task}` ]
      then
        value="-"
      else
        fileName=`OrderTarget "${combi}-${task}"`
        targetName=`FullTargetName ${task}`
        value=`cat ${prefix}${fileName}.*.$targetName | grep "^4" | cut -f 3`
        value=${value:0:6}
        if [ -z $value ]
        then
          value="N/A"
        else
          if [ `CompareToPreviousScore ${task} ${value}` ]
          then
            value="\cellcolor[gray]{0.85} ${value}"
          fi
        fi
      fi

      echo -n " & ${value}"
    done
  
    echo " \\\\"
  done
}

prefix=$1

InitScores

#\cellcolor[gray]{0.85}
# Header
echo "\documentclass{article}"
echo "\usepackage{tabularx,colortbl,amsmath,longtable}"
echo "\usepackage{fullpage}"
echo "\addtolength{\oddsidemargin}{-.875in}"
echo "\addtolength{\evensidemargin}{-.875in}"
echo "\addtolength{\textwidth}{1.75in}"
#echo "\addtolength{\topmargin}{-.875in}"
#echo "\addtolength{\textheight}{1.75in}"
echo "\begin{document}"
echo "\title{${2}}"
echo "\author{Francis Maes, Julien Becker}"
echo "\maketitle"
echo "\begin{longtable}{l||cccccc}"
echo "   & \multicolumn{6}{c}{{\bf Target}} \\\\"
echo "  {\bf Method} & SS3 & SS8 & SA & DR & BBB & StAl \\\\ \hline \hline \endhead"
echo "  \hline \multicolumn{7}{r}{{Continued on next page}} \\ \endfoot"
echo "  \endlastfoot"
echo "  \hline \hline"

# Content-Only
echo -n "  Content-Only"
for task in $TARGETS
do
  echo -n " & "
  targetName=`FullTargetName ${task}`
  value=`cat ${prefix}${task}.*.${targetName} | grep "^0" | cut -f 3`

  setCurrentScore ${task} $value
  if [ -z $value ]
  then
    value="N/A"
    setCurrentScore ${task} $_MINUS_INF
  fi

  echo -n ${value:0:6}
done
echo " \\\\"
echo "  \hline \hline"

FromCurrentToPreviousScores

# Muliple Passes
for pass in 1 4 9
do
  userPass=`echo "${pass} + 1" | bc`
  echo -n "  Multi-Pass ${userPass}"
  for task in $TARGETS
  do
    echo -n " & "
    targetName=`FullTargetName ${task}`
    value=`cat ${prefix}${task}.*.${targetName} | grep "^${pass}" | cut -f 3`
    value=${value:0:6}
    if [ -z $value ]
    then
      value="N/A"
    elif [ `CompareToPreviousScore ${task} ${value}` ]
    then
      value="\cellcolor[gray]{0.85} ${value}"
    fi
    echo -n ${value}
  done
  echo " \\\\"
done
echo "  \hline \hline"

FromCurrentToPreviousScores

# Multi-Task
for (( nbTask=1; nbTask <= `echo ${TARGETS} | wc -w`; nbTask++ ))
do
  MultiTask `expr ${nbTask} - 1`
  echo "  \hline"

  FromCurrentToPreviousScores
done

# Footer
echo "\end{longtable}"
echo "\end{document}"

rm .currentScore.*
rm .previousScore.*

