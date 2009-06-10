/*-----------------------------------------.---------------------------------.
| Filename: ClassificationToBatchRanking.cpp| An example to show how to use  |
| Author  : Francis Maes                   |  ranking machines to solve      |
| Started : 09/06/2009 18:48               |  classification.                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;


/*
** FIXME: cet exemple ne marche pas encore
*/
double evaluateAccuracy(ObjectContainerPtr trainingData, ObjectContainerPtr testingData, RankerPtr ranker)
{
  ranker->trainBatch(trainingData);
  return 1.0 - ranker->evaluateMeanTopRankCost(testingData);
}

int main(int argc, char* argv[])
{
  /*
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();
  
  /*
  ** Create parser and apply the classification example -> ranking example function.
  */
  ObjectFunctionPtr conversionFunction = transformClassificationExampleIntoRankingExample(labels);
  ObjectStreamPtr trainingParser = classificationExamplesParser("../data/classification/small.train", features, labels);
  ObjectContainerPtr trainingData = trainingParser->apply(conversionFunction)->load();
  return 0.0;
  ObjectStreamPtr testingParser = classificationExamplesParser("../data/classification/small.test", features, labels);
  ObjectContainerPtr testingData = testingParser->apply(conversionFunction)->load();
  if (trainingData->empty() || testingData->empty())
    return 1.0;
  
  /*
  ** Create ranking machines
  */
  std::vector<std::pair<std::string, RankerPtr> > rankers;
  
  GradientBasedLearnerPtr gradientDescentLearner =
    batchLearner(gradientDescentOptimizer(constantIterationFunction(0.0)));
  GradientBasedLearnerPtr rpropLearner = batchLearner(rpropOptimizer());
  
  rankers.push_back(std::make_pair("AllPairs, Gradient Descent",
    largeMarginAllPairsLinearRanker(gradientDescentLearner)));
  rankers.push_back(std::make_pair("MostViolatedPair, Gradient Descent",
    largeMarginMostViolatedPairLinearRanker(gradientDescentLearner)));
  rankers.push_back(std::make_pair("BestAgainstAll, Gradient Descent",
    largeMarginBestAgainstAllLinearRanker(gradientDescentLearner)));
    
  rankers.push_back(std::make_pair("AllPairs, RProp",
    largeMarginAllPairsLinearRanker(rpropLearner)));
  rankers.push_back(std::make_pair("MostViolatedPair, RProp",
    largeMarginMostViolatedPairLinearRanker(rpropLearner)));
  rankers.push_back(std::make_pair("BestAgainstAll, RProp",
    largeMarginBestAgainstAllLinearRanker(rpropLearner)));
  
  /*
  ** Evaluate each ranking machine
  */
  for (size_t i = 0; i < rankers.size(); ++i)
    std::cout << "Ranker '" << rankers[i].first
      << "' accuracy = " << evaluateAccuracy(trainingData, testingData, rankers[i].second) * 100.0 << "%"
      << std::endl;
  return 0;
}
