/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearningMachine.cpp| Gradient based                 |
| Author  : Francis Maes                   |        learning machines        |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/GradientBasedLearningMachine.h>
#include "GradientBasedLearningMachine/GradientBasedRegressor.h"
#include "GradientBasedLearningMachine/GradientBasedClassifier.h"
#include "GradientBasedLearningMachine/GradientBasedBinaryClassifier.h"
#include "GradientBasedLearningMachine/GradientBasedRanker.h"
using namespace lbcpp;

/*
** Gradient Based Learning Machine
*/
void GradientBasedLearningMachine::saveImpl(std::ostream& ostr) const
{
  FeatureDictionary::save(ostr, inputDictionary, parameters->getDictionary());
  parameters->save(ostr);
  write(ostr, regularizer);
  write(ostr, learner);
  write(ostr, initializeParametersRandomly);
}

bool GradientBasedLearningMachine::loadImpl(std::istream& istr)
{
  FeatureDictionaryPtr paramsDictionary;
  if (!FeatureDictionary::load(istr, inputDictionary, paramsDictionary))
    return false;
  parameters = new DenseVector(paramsDictionary);
  return parameters->load(istr) &&
            read(istr, regularizer) &&
            read(istr, learner) &&
            read(istr, initializeParametersRandomly);
}

void GradientBasedLearningMachine::cloneImpl(GradientBasedLearningMachine& target) const
{
  target.inputDictionary = inputDictionary;
  target.parameters = parameters ? parameters->cloneAndCast<DenseVector>() : DenseVectorPtr();
  target.regularizer = regularizer;
  target.learner = learner->cloneAndCast<GradientBasedLearner>();
  target.initializeParametersRandomly = initializeParametersRandomly;
}


void GradientBasedLearningMachine::trainStochasticBeginImpl(FeatureDictionaryPtr inputDictionary)
{
  assert(learner);
  if (!parameters && inputDictionary)
    createParameters(inputDictionary, initializeParametersRandomly);
  learner->setParameters(parameters);
  learner->setRegularizer(getRegularizer());
  learner->trainStochasticBegin(inputDictionary);
}
  
void GradientBasedLearningMachine::trainStochasticExampleImpl(FeatureGeneratorPtr gradient, double weight)
{
  assert(learner);
  learner->setParameters(parameters);
  learner->trainStochasticExample(gradient, weight);
}

void GradientBasedLearningMachine::trainStochasticExampleImpl(ObjectPtr example)
{
  assert(learner);
  if (!parameters)
    createParameters(getInputDictionaryFromExample(example), initializeParametersRandomly);
  learner->setParameters(parameters);
  learner->trainStochasticExample(example, getLoss(example));
}

void GradientBasedLearningMachine::trainStochasticEndImpl()
{
  assert(learner);
  learner->trainStochasticEnd();
}

bool GradientBasedLearningMachine::trainBatchImpl(ObjectContainerPtr examples, ProgressCallbackPtr progress)
{
  assert(learner && examples->size());
  if (!parameters)
    createParameters(getInputDictionaryFromExample(examples->get(0)), initializeParametersRandomly);

  // delegate to learner
  learner->setParameters(parameters);
  learner->setRegularizer(getRegularizer());
  if (!learner->trainBatch(getRegularizedEmpiricalRisk(examples), examples->size(), progress))
    return false;
  parameters = learner->getParameters();
  return true;
}

/*
** Regressor
*/
GradientBasedRegressorPtr lbcpp::leastSquaresLinearRegressor(GradientBasedLearnerPtr learner)
{
  GradientBasedRegressorPtr res = new LeastSquaresLinearRegressor();
  res->setLearner(learner);
  return res;
}

/*
** Classifier
*/
GradientBasedClassifierPtr lbcpp::maximumEntropyClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer)
{
  GradientBasedClassifierPtr res = new MaximumEntropyClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  if (l2regularizer)
    res->setL2Regularizer(l2regularizer);
  return res;
}

/*
** BinaryClassifier
*/
GradientBasedBinaryClassifierPtr lbcpp::logisticRegressionBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer)
{
  GradientBasedBinaryClassifierPtr res = new LogisticRegressionClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  if (l2regularizer)
    res->setL2Regularizer(l2regularizer);
  return res;
}

GradientBasedBinaryClassifierPtr lbcpp::linearSVMBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels)
{
  GradientBasedBinaryClassifierPtr res = new LinearSupportVectorMachine();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}

/*
** Generalized classifier
*/
GradientBasedGeneralizedClassifierPtr lbcpp::linearGeneralizedClassifier(GradientBasedLearnerPtr learner)
{
  GradientBasedGeneralizedClassifierPtr res = new LinearGeneralizedClassifier();
  res->setLearner(learner);
  return res;
}

/*
** Ranker
*/
GradientBasedRankerPtr lbcpp::largeMarginAllPairsLinearRanker(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginAllPairsLinearRanker();
  res->setLearner(learner);
  return res;
}

GradientBasedRankerPtr lbcpp::largeMarginBestAgainstAllLinearRanker(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginBestAgainstAllLinearRanker();
  res->setLearner(learner);
  return res;
}

GradientBasedRankerPtr lbcpp::largeMarginMostViolatedPairLinearRanker(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginMostViolatedPairLinearRanker();
  res->setLearner(learner);
  return res;
}

GradientBasedRankerPtr lbcpp::logBinomialAllPairsLinearRanker(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LogBinomialAllPairsLinearRanker();
  res->setLearner(learner);
  return res;
}

/*
** Serializable classes declaration
*/
void declareGradientBasedLearningMachines()
{
  LBCPP_DECLARE_CLASS(LeastSquaresLinearRegressor);
  LBCPP_DECLARE_CLASS(MaximumEntropyClassifier);
  LBCPP_DECLARE_CLASS(LogisticRegressionClassifier);
  LBCPP_DECLARE_CLASS(LinearSupportVectorMachine);
  LBCPP_DECLARE_CLASS(LinearGeneralizedClassifier);
  LBCPP_DECLARE_CLASS(LargeMarginAllPairsLinearRanker);
  LBCPP_DECLARE_CLASS(LargeMarginBestAgainstAllLinearRanker);
  LBCPP_DECLARE_CLASS(LargeMarginMostViolatedPairLinearRanker);
  LBCPP_DECLARE_CLASS(LogBinomialAllPairsLinearRanker);
}
