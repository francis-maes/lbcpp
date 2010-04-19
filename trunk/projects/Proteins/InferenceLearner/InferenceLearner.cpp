/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearner.cpp           | Inference Learners              |
| Author  : Francis Maes                   |                                 |
| Started : 11/04/2010 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GlobalSimulationInferenceLearner.h"
#include "SingleStepDeterministicSimulationInferenceLearner.h"
using namespace lbcpp;

InferenceLearnerPtr lbcpp::globalSimulationLearner(InferenceLearnerCallbackPtr callback)
  {return new GlobalSimulationInferenceLearner(callback);}

InferenceLearnerPtr lbcpp::stepByStepDeterministicSimulationLearner(InferenceLearnerCallbackPtr callback, bool useCacheOnTrainingData, size_t firstStepToLearn)
  {return new StepByStepDeterministicSimulationLearner(callback, useCacheOnTrainingData, firstStepToLearn);}
