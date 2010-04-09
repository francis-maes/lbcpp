/*-----------------------------------------.---------------------------------.
| Filename: LearningExample.cpp            | Learning examples               |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/LearningExample.h>
#include <lbcpp/ObjectStream.h>
#include <lbcpp/SparseVector.h>
using namespace lbcpp;

class TransformClassificationExampleIntoRankingExample : public ObjectFunction
{
public:
  TransformClassificationExampleIntoRankingExample(FeatureDictionaryPtr labels) 
    : labels(labels) {}
  
  virtual String getOutputClassName(const String& inputClassName) const
    {return "RankingExample";}
    
  virtual ObjectPtr function(ObjectPtr object) const
  {
    ClassificationExamplePtr classificationExample = object.dynamicCast<ClassificationExample>();
    FeatureGeneratorPtr input = classificationExample->getInput();
    size_t output = classificationExample->getOutput();
    
    FeatureDictionaryPtr dictionary2 = FeatureDictionaryManager::getInstance().getCollectionDictionary(labels->getFeatures(), input->getDictionary());
    FeatureDictionaryPtr dictionary = FeatureDictionaryManager::getInstance().getCollectionDictionary(labels->getFeatures(), dictionary2);

    size_t numLabels = labels->getNumFeatures();
    CompositeFeatureGeneratorPtr alternatives = new CompositeFeatureGenerator(dictionary, numLabels);
    std::vector<double> costs(numLabels);
    for (size_t i = 0; i < numLabels; ++i)
    {
      alternatives->setSubGenerator(i, subFeatureGenerator(dictionary2, i, input));
      costs[i] = (i == output ? 0.0 : 1.0);
    }
    return new RankingExample(alternatives, costs);
  }

private:
  FeatureDictionaryPtr labels;
};

ObjectFunctionPtr lbcpp::transformClassificationExampleIntoRankingExample(FeatureDictionaryPtr labels)
{
  return new TransformClassificationExampleIntoRankingExample(labels);
}
