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

class VerboseFeatureVisitor : public FeatureVisitor
{
public:
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
  {
    std::cout << "Enter(" << dictionary->getScopes()->getString(index) << ")" << std::endl;
    return true;
  }
  
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
  {
    std::cout << "." << std::flush;
  }

  virtual void featureLeave()
  {
    std::cout << "Leave()" << std::endl;
  }
};

class TransformClassificationExampleIntoRankingExample : public ObjectFunction
{
public:
  TransformClassificationExampleIntoRankingExample(StringDictionaryPtr labels) 
    : labels(labels), dictionary(new FeatureDictionary("alternatives", StringDictionaryPtr(), labels)),
      dictionary2(new FeatureDictionary("alternative", StringDictionaryPtr(), labels)) {}
  
  StringDictionaryPtr labels;
  FeatureDictionaryPtr dictionary;
  FeatureDictionaryPtr dictionary2;

  virtual std::string getOutputClassName() const
    {return "RankingExample";}
    
  virtual ObjectPtr function(ObjectPtr object) const
  {
    ClassificationExamplePtr classificationExample = object.dynamicCast<ClassificationExample>();
    FeatureGeneratorPtr input = classificationExample->getInput();
    size_t output = classificationExample->getOutput();
    
    size_t numLabels = labels->getNumElements();
    CompositeFeatureGeneratorPtr alternatives = new CompositeFeatureGenerator(dictionary, numLabels);
    std::vector<double> costs(numLabels);
    for (size_t i = 0; i < numLabels; ++i)
    {
      alternatives->setSubGenerator(i, subFeatureGenerator(dictionary2, i, input));
      dictionary->setSubDictionary(i, input->getDictionary());
      costs[i] = (i == output ? 0.0 : 1.0);
    }
    
    static int pouet = 0;
    ++pouet;
    if (pouet == 5)
    {
      
      FeatureVisitorPtr verbose = new VerboseFeatureVisitor();
//      std::cout << " ===== ALTERNATIVES1 ===== " << std::endl;
//      alternatives->accept(verbose);
      std::cout << std::endl << " ===== ALTERNATIVES2 ===== " << std::endl;
      std::cout << alternatives->toString() << std::endl;
//      std::cout << std::endl << " ===== ALTERNATIVES1b ===== " << std::endl;
//      alternatives->toSparseVector()->accept(verbose);
//      std::cout << std::endl << " ===== ALTERNATIVES2b ===== " << std::endl;
//      std::cout << alternatives->toSparseVector()->toString() << std::endl;
//      std::cout << std::endl << std::endl;
      exit(0);
    }
    
//    std::cout << "ALTERNATIVES vec: " << alternatives->toSparseVector()->toString() << std::endl;
    return new RankingExample(alternatives, costs);
  }
};

ObjectFunctionPtr lbcpp::transformClassificationExampleIntoRankingExample(StringDictionaryPtr labels)
{
  return new TransformClassificationExampleIntoRankingExample(labels);
}
