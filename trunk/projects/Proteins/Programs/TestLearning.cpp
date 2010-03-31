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

StoppingCriterionPtr createLearningStoppingCriterion()
{
  return logicalOr(maxIterationsStoppingCriterion(10), maxIterationsWithoutImprovementStoppingCriterion(5));
}

GradientBasedClassifierPtr createMaxentClassifier(StringDictionaryPtr labels, bool batchLearner = true)
{
  IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
  GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);
  if (batchLearner)
    learner = stochasticToBatchLearner(learner, createLearningStoppingCriterion());
  
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
  classifier->setL2Regularizer(0.0001);
  return classifier;
}

  ObjectContainerPtr makeClassificationExamples(ObjectContainerPtr examples)
  {
    VectorObjectContainerPtr res = new VectorObjectContainer("ClassificationExample");
    for (size_t i = 0; i < examples->size(); ++i)
    {
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      jassert(example);
      VariableSetPtr targetVariables = example->getTargetVariables();
      size_t n = example->getNumVariables();
      for (size_t j = 0; j < n; ++j)
      {
        size_t value;
        if (targetVariables->getVariable(j, value))
          res->append(new ClassificationExample(example->getVariableFeatures(j), value));
      }
    }
    return res;
  }  

int main()
{
  File cb513Directory = 
    //File(T("/Users/francis/Projets/Proteins/Data/CB513"));
    File(T("C:/Projets/Proteins/data/CB513cool"));

  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(cb513Directory, T("*.protein"));
  ObjectStreamPtr examplesStream = proteinsStream->apply(new ProteinToVariableSetExample());
  ObjectContainerPtr examples = examplesStream->load()->randomize();
  StringDictionaryPtr labels = examples->getAndCast<VariableSetExample>(0)->getTargetVariables()->getVariablesDictionary();


  size_t numFolds = 7;
  double cvTrainResult = 0.0, cvTestResult = 0.0;
  for (size_t i = 0; i < numFolds; ++i)
  {
    VariableSetModelPtr model =
      //independantClassificationVariableSetModel(createMaxentClassifier(labels));
      //optimisticClassificationVariableSetModel(createMaxentClassifier(labels))

      //iterativeClassificationVariableSetModel(createMaxentClassifier(labels), createMaxentClassifier(labels));
      simulatedIterativeClassificationVariableSetModel(createMaxentClassifier(labels, false), createLearningStoppingCriterion());

    std::cout << std::endl << std::endl << "FOLD " << (i+1) << " / " << numFolds << "...." << std::endl;
    model->trainBatch(examples->invFold(i, numFolds), consoleProgressCallback());
    double trainAccuracy = model->evaluate(examples->invFold(i, numFolds));
    double testAccuracy = model->evaluate(examples->fold(i, numFolds));
    std::cout << "Train Score: " << trainAccuracy << std::endl;
    std::cout << "Test Score: " << testAccuracy << std::endl;
    cvTrainResult += trainAccuracy;
    cvTestResult += testAccuracy;
  }
  
  //std::cout << "Average Train Accuracy = " << cvTrainResult / numFolds << std::endl;
  //std::cout << "Average Test Accuracy = " << cvTestResult / numFolds << std::endl;
 
  // Results: Prediction of SS3 / 7 folds CV
  
  // ---CO---
  //
  // AA(7)+PSSM(7) with 1/1 train iter => 71.30
  // AA(7)+PSSM(7) with 2/2 train iter => 72.01
  // AA(7)+PSSM(7) with 2/10 train iter => 72.29
 
  
  // ---ICA---
  // AA(7)+PSSM(7) + PR(1) with 2/2 train iter => 70.96
  // AA(7)+PSSM(7) + PR(2) with 2/2 train iter => 71.67
  // AA(7)+PSSM(7) + PR(5) with 2/2 train iter => 71.63
  
  // ---SICA---
  // AA(7)+PSSM(7) + PR(2) with 2/2 train iter, reg 0, maxpass 5 => 70.61
  // AA(7)+PSSM(7) + PR(2) with 2/10 train iter, reg 0.0001, maxpass 10
  
  // ---CO Results with only 1 folds over the seven, 10 training iteration, best test score---
  // AA(7)+PSSM(7) => 74.02
  // AA(7)+PSSM(7)+POS/LEN => 74.16
  // AA(7)+PSSM(7)+POS/LEN/CONJ(AA) => 74.19
  // AA(7)+PSSM(7)+POS%/LEN/CONJ(AA) => 74.07
  // AA(7)+PSSM(7)+POS/LEN/CONJ(AA)/CONJ(PSSM) => 74.00

  // ---Test Results on the whole dataset (Predition of SS3)---
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
