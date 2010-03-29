/*-----------------------------------------.---------------------------------.
| Filename: TestLearning.cpp               | Test Learning                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2010 12:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/Protein.lh"
#include "../VariableSetModel.h"
using namespace lbcpp;

extern void declareProteinsClasses();
extern void declareInterdependantVariableSetClasses();

class ProteinToVariableSetExample : public ObjectFunction
{
public:
  virtual String getOutputClassName(const String& inputClassName) const
    {return T("VariableSetExample");}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    return new SecondaryStructureVariableSetExample(protein);
  }
};

ClassifierPtr createMaxentClassifier(StringDictionaryPtr labels)
{
  IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
  GradientBasedLearnerPtr learner = stochasticToBatchLearner(stochasticDescentLearner(learningRate), 2, 2);
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
  classifier->setL2Regularizer(0.001);
  return classifier;
}

int main()
{
  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(File(T("/Users/francis/Projets/Proteins/Data/CB513")), T("*.protein"));
  ObjectStreamPtr examplesStream = proteinsStream->apply(new ProteinToVariableSetExample());
  ObjectContainerPtr examples = examplesStream->load()->randomize();
  StringDictionaryPtr labels = examples->getAndCast<VariableSetExample>(0)->getTargetVariables()->getVariablesDictionary();

  size_t numFolds = 7;
  double cvResult = 0.0;
  for (size_t i = 0; i < numFolds; ++i)
  {
    VariableSetModelPtr model =
      //new IndependantClassificationVariableSetModel(createMaxentClassifier(labels));
      iterativeClassificationVariableSetModel(createMaxentClassifier(labels), createMaxentClassifier(labels));

    std::cout << std::endl << std::endl << "FOLD " << (i+1) << " / " << numFolds << "...." << std::endl;
    model->trainBatch(examples->invFold(i, numFolds), consoleProgressCallback());
    double trainAccuracy = model->evaluate(examples->invFold(i, numFolds));
    double testAccuracy = model->evaluate(examples->fold(i, numFolds));
    std::cout << "Train Score: " << trainAccuracy << std::endl;
    std::cout << "Test Score: " << testAccuracy << std::endl;
    cvResult += testAccuracy;
  }
  
  std::cout << std::endl << std::endl << "Cross Validation Result: " << (cvResult / numFolds) << std::endl;
 
  // Results: Prediction of SS3 / 7 folds CV
  
  // ---CO---
  //
  // AA(7)+PSSM(7) with 1/1 train iter => 71.30
  // AA(7)+PSSM(7) with 2/2 train iter => 72.01
  // AA(7)+PSSM(7) with 2/10 train iter => 72.29
  
  // ---ICA---
  // AA(7)+PSSM(7) with 2/2 train iter => 71.67
  
  // Results: Predition of SS3:
  // AA(15) => 62.2
  // PSSM(15) => 72.1
  // AA(15)+PSSM(15) => 73.9
  // AA(15)+PSSM(15)+OPT(1) => 88.3
  // AA(15)+PSSM(15)+OPT(10) => 89.1
  // AA(15)+PSSM(15)+OPT_8(10) => 90.5
  // PSSM(15)+OPT(1) => 87.8
  // AA(15)+OPT(1) => 87.6
  // OPT(1) => 82.8
  // OPT(10) => 84.7
  return 0;
}
