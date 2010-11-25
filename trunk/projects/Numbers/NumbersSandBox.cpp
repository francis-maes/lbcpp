/*-----------------------------------------.---------------------------------.
| Filename: NumbersSandBox.cpp             | Numbers Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2010 18:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../../src/Data/Object/DenseDoubleObject.h"
#include "NumberPerceptions.h"
using namespace lbcpp;

InferenceOnlineLearnerPtr createOnlineLearner()
{
  InferenceOnlineLearnerPtr res, lastLearner;
  
  res = randomizerOnlineLearner(perPass);
  res->setNextLearner(lastLearner = gradientDescentOnlineLearner(
      perStep, invLinearIterationFunction(1.0, 1000), true, // learning steps
      perStep, l2RegularizerFunction(0.01)));         // regularizer

  lastLearner->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(10), true)); // stopping criterion
  return res;
}

PerceptionPtr createPerception()
{
  CompositePerceptionPtr res = new CompositePerception(numbersSerieProblemClass, T("numbers-serie"));
  FunctionPtr problemToIntegerPair = new NumbersSerieProblemGetNumbersPairFunction();
  res->addPerception(T("bidirectional"), composePerception(problemToIntegerPair, pairBidirectionalPerception(numberPairIsMultipleFeatures(2, 5))));
  res->addPerception(T("difference"), composePerception(problemToIntegerPair,
    numberPairDifferencePerception(twoDigitNumberFeatures())));
  return res;
}

InferencePtr createRankingInference(PerceptionPtr perception)
{
  NumericalSupervisedInferencePtr res = allPairsRankingLinearSVMInference(T("numbers-ranker"), perception);
  res->setStochasticLearner(createOnlineLearner());
  return res;
}

////////////////////////


ContainerPtr parseNumberSequence(const String& str)
{
  StringArray tokens;
  tokens.addTokens(str, true);
  ContainerPtr res = vector(integerType, tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
    res->setElement(i, tokens[i].getIntValue());
  return res;
}

ContainerPtr sampleNumberSequence(RandomGeneratorPtr random, int minValue = 0, int maxValue = 100, size_t length = 6)
{
  ContainerPtr res = vector(integerType, length);
  for (size_t i = 0; i < length; ++i)
    res->setElement(i, random->sampleInt(minValue, maxValue));
  return res;
}

ContainerPtr sampleNumberSequences(RandomGeneratorPtr random, int minValue = 0, int maxValue = 100, size_t length = 6, size_t count = 10000)
{
  ContainerPtr res = vector(enrichedNumberSequenceClass, count);
  for (size_t i = 0; i < count; ++i)
    res->setElement(i, new EnrichedNumberSequence(sampleNumberSequence(random, minValue, maxValue, length)));
  return res;
}


double factorial(double value)
{
  double res = 1.0;
  for (double i = 2.0; i <= value; ++i)
    res *= i;
  return res;
}
/*
double binomialProbability(double numSuccess, double numTrials, double probability)
{
  size_t numFailures = numTrials - numSuccess;
  return factorial(numTrials) / (factorial(numSuccess) * factorial(numFailures))
                  * pow(probability, numSuccess) * pow(1 - probability, numFailures);
}*/

/*double featureScore(double featureValue, size_t numExamples, double featureExpectation)
{
  return log(binomialProbability(featureValue, 
}*/

extern void declareNumbersClasses(ExecutionContext& context);

ObjectPtr estimateFeaturesExpectation(ExecutionContext& context, PerceptionPtr perception, ContainerPtr inputs)
{
  if (!context.checkInheritance(inputs->getElementsType(), perception->getInputType()))
    return ObjectPtr();

  size_t n = inputs->getNumElements();
  ObjectPtr mean;
  double invK = 1.0 / (double)n;
  for (size_t i = 0; i < n; ++i)
    lbcpp::addWeighted(context, mean, perception, inputs->getElement(i), invK);
  return mean;
}

class FeaturesInformation
{
public:
  FeaturesInformation(PerceptionPtr perception)
    : perception(perception), statistics(perception->getNumOutputVariables()), activeFeatures(perception->getNumOutputVariables())
  {
    for (size_t i = 0; i < statistics.size(); ++i)
    {
      statistics[i].setName(perception->getOutputVariableName(i));
      statistics[i].setStaticAllocationFlag();
      activeFeatures[i].setName(perception->getOutputVariableName(i));
    }
  }

  void compute(ExecutionContext& context, ContainerPtr inputs)
  {
    size_t n = inputs->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr target;
      lbcpp::addWeighted(context, target, perception, inputs->getElement(i), 1.0);
      DenseDoubleObjectPtr denseTarget = target.dynamicCast<DenseDoubleObject>();
      jassert(denseTarget);
      update(denseTarget->getValues());
    }
  }

  void print()
  {
    for (size_t i = 0; i < statistics.size(); ++i)
      std::cout << statistics[i].toString() << std::endl;
  }

  void getRelevanceScores(ExecutionContext& context, const Variable& input, std::multimap<double, size_t>& featuresByRelevance)
  {
    ObjectPtr object;
    lbcpp::addWeighted(context, object, perception, input, 1.0);
    DenseDoubleObjectPtr denseObject = object.staticCast<DenseDoubleObject>();
    std::vector<double>& featureValues = denseObject->getValues();
    
    jassert(featureValues.size() == statistics.size());
    for (size_t i = 0; i < featureValues.size(); ++i)
    {
      if (doubleType->isMissingValue(featureValues[i]))
        continue;
      double featureScore = getFeatureScore(i, featureValues[i]);
      featuresByRelevance.insert(std::make_pair(featureScore, i));
    }
  }

  void printMostRelevantFeatures(ExecutionContext& context, const Variable& input)
  {
    typedef std::multimap<double, size_t> FeaturesByScoreMap;
    FeaturesByScoreMap featuresByRelevance;
    getRelevanceScores(context, input, featuresByRelevance);
    int i = 0;
    for (FeaturesByScoreMap::const_reverse_iterator it = featuresByRelevance.rbegin(); it != featuresByRelevance.rend() && i < 10; ++it, ++i)
      std::cout << "Top " << (i+1) << ": " << perception->getOutputVariableName(it->second) << " (" << it->first << ")" << std::endl;
  }

private:
  PerceptionPtr perception;
  std::vector<ScalarVariableStatistics> statistics;
  std::vector<ScalarVariableMean> activeFeatures; 

  double getAPrioriProbability(size_t index, double value) const
  {
    return activeFeatures[index].getMean();

    const ScalarVariableStatistics& stats = statistics[index];
    if (!stats.getCount())
      return 0.0;

    double v = stats.getVariance();
    if (v == 0)
      return value == stats.getMean() ? 1.0 : 0.0;
    else
      return exp(- (value - stats.getMean()) * (value - stats.getMean()) / (2.0 * v)) / sqrt(2.0 * M_PI * v);
  }

  double getFeatureScore(size_t index, double value) const
  {
    double p = getAPrioriProbability(index, value);
    return p ? -log(p) * value : DBL_MAX;
  }

  void update(const std::vector<double>& values)
  {
    jassert(values.size() == statistics.size());
    for (size_t i = 0; i < values.size(); ++i)
    {
      double value = doubleType->isMissingValue(values[i]) ? 0.0 : values[i];
      statistics[i].push(value);
      activeFeatures[i].push(value > 0.0);
    }
  }
};


int main(int argc, char* argv[])
{
  lbcpp::initialize();
  ExecutionContextPtr context = defaultConsoleExecutionContext();

  declareNumbersClasses(*context);

  // Perception
  PerceptionPtr numberPerception = twoDigitNumberFeatures();
  PerceptionPtr numberPairPerception = pairBidirectionalPerception(numberPairIsMultipleFeatures(2, 10));
  PerceptionPtr numberTripletPerception = nullPerception();

  CompositePerceptionPtr numbersPerception = new CompositePerception(containerClass(doubleType), T("numbers"));
  numbersPerception->addPerception(T("isConstant"), new ContainerIsConstantFeature());
  numbersPerception->addPerception(T("sum"), containerSumFeatures(numberPerception));

  PerceptionPtr pairsPerception = containerSumFeatures(numberPairPerception);
  PerceptionPtr tripletsPerception = containerSumFeatures(numberTripletPerception);
  PerceptionPtr perception = flattenPerception(enrichedNumberSequencePerception(numbersPerception, pairsPerception, tripletsPerception));

  // Random Number Sequences
  ContainerPtr randomInputs = sampleNumberSequences(RandomGenerator::getInstance(), 0, 100, 6, 10000);
  FeaturesInformation featuresInfo(perception);
  featuresInfo.compute(*context, randomInputs);
  //featuresInfo.print();

  ContainerPtr problem = parseNumberSequence(T("1 2 4 8 16"));
  EnrichedNumberSequencePtr enrichedProblem = new EnrichedNumberSequence(problem);
  featuresInfo.printMostRelevantFeatures(*context, enrichedProblem);
  return 0;

  /*
  ContainerPtr trainingData = problem->createRankingDataForAllSubProblems();
  Variable testingExample = problem->createRankingExample();
  
  PerceptionPtr perception = createPerception();
  Variable perceived = perception->compute(testingExample.getObjectAndCast<Pair>()->getFirst()[32]);
  perceived.printRecursively(std::cout, -1, false, false);
  //return 0;
  
  std::cout << trainingData->getNumElements() << " training examples" << std::endl;

  InferencePtr rankingInference = createRankingInference(perception);

  InferenceContextPtr context = singleThreadedInferenceContext();

  std::cout << "Training..." << std::endl;
  context->train(rankingInference, trainingData);

  std::cout << "Testing: " << std::endl;
  ContainerPtr predictedScores = context->predict(rankingInference, testingExample[0]).getObjectAndCast<Container>();
  //predictedScores.printRecursively(std::cout);

  std::set<int> bestNumbers;
  double bestScore = -DBL_MAX;
  for (int i = 0; i < 100; ++i)
  {
    double score = predictedScores->getElement(i).getDouble();
    if (score >= bestScore)
    {
      if (score > bestScore)
      {
        bestNumbers.clear();
        bestScore = score;
      }
      bestNumbers.insert(i);
    }
  }
  std::cout << "Best Numbers: ";
  for (std::set<int>::const_iterator it = bestNumbers.begin(); it != bestNumbers.end(); ++it)
    std::cout << *it << " ";
  std::cout << std::endl;*/
  return 0;
}
