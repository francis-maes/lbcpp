/*-----------------------------------------.---------------------------------.
| Filename: NumbersSandBox.cpp             | Numbers Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2010 18:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "NumberPerceptions.h"
using namespace lbcpp;

InferenceOnlineLearnerPtr createOnlineLearner()
{
  InferenceOnlineLearnerPtr res = gradientDescentOnlineLearner(
      InferenceOnlineLearner::perPass, //perStepMiniBatch1000,                                                 // randomization
      InferenceOnlineLearner::perStep, invLinearIterationFunction(1.0, 1000), true, // learning steps
      InferenceOnlineLearner::perStep, l2RegularizerFunction(0.01));         // regularizer

  res->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(InferenceOnlineLearner::perPass,
        maxIterationsStoppingCriterion(10), true)); // stopping criterion
  return res;
}

PerceptionPtr createPerception()
{
  CompositePerceptionPtr res = new CompositePerception(numbersSerieProblemClass, T("numbers-serie"));
  res->addPerception(T("bidirectional"), composePerception(new NumbersSerieProblemGetNumbersPairFunction(),
              pairBidirectionalPerception(numberPairIsMultipleFeatures(2, 5))));
  return res;
}

InferencePtr createRankingInference()
  {return allPairsRankingLinearSVMInference(createPerception(), createOnlineLearner());}

extern void declareNumbersClasses();

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareNumbersClasses();

  std::vector<int> numbers; 
  NumbersSerieProblemPtr problem = new NumbersSerieProblem(T("1 2 4 8 16 32"));

  ContainerPtr trainingData = problem->createRankingDataForAllSubProblems();
  Variable testingExample = problem->createRankingExample();
  
  //PerceptionPtr testPerception = createPerception();
  //Variable perceived = testPerception->compute(testingExample.getObjectAndCast<Pair>()->getFirst()[32]);
  //perceived.printRecursively(std::cout, -1, false, false);
  
  std::cout << trainingData->getNumElements() << " training examples" << std::endl;

  InferencePtr rankingInference = createRankingInference();

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
  std::cout << std::endl;
  return 0;
}
