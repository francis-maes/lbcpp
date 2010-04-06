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
  return maxIterationsStoppingCriterion(100);
}

class TestTrainingProgressCallback : public TrainingProgressCallback
{
public:
  TestTrainingProgressCallback(StoppingCriterionPtr stoppingCriterion, ObjectContainerPtr validationData)
    : trainAccuracy(0.0), testAccuracy(0.0), stoppingCriterion(stoppingCriterion), validationData(validationData) {}
  
  double trainAccuracy;
  double testAccuracy;
  
  virtual void progressStart(const String& description)
  {
    std::cout << description << std::endl;
    stoppingCriterion->reset();
  }
  
  virtual bool trainingProgressStep(LearningMachinePtr m, ObjectContainerPtr trainingData)
  {
    VariableSetModelPtr model = m.dynamicCast<VariableSetModel>();
    jassert(model);
    
    std::cout << "Evaluating on training data..." << std::flush;
    trainAccuracy = model->evaluate(trainingData);
    std::cout << " => " << trainAccuracy << std::endl;

    std::cout << "Evaluating on testing data..." << std::flush;
    testAccuracy = model->evaluate(validationData);
    std::cout << " => " << testAccuracy << std::endl;
    
    return !stoppingCriterion->shouldOptimizerStop(trainAccuracy);
  }

  virtual void progressEnd()
    {std::cout << "Training finished." << std::endl;}
    
private:
  StoppingCriterionPtr stoppingCriterion;
  ObjectContainerPtr validationData;
};

GradientBasedClassifierPtr createMaxentClassifier(StringDictionaryPtr labels)
{
  IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
  GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
  classifier->setL2Regularizer(0.0001);
  return classifier;
}

int main()
{
  File cb513Directory = 
    File(T("/Users/francis/Projets/Proteins/Data/CB513"));
    //File(T("C:/Projets/Proteins/data/CB513cool"));

  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(cb513Directory, T("*.protein"));
  ObjectStreamPtr examplesStream = proteinsStream->apply(new ProteinToVariableSetExample());
  ObjectContainerPtr examples = examplesStream->load()->randomize()->fold(0, 30);
  StringDictionaryPtr labels = examples->getAndCast<VariableSetExample>(0)->getTargetVariables()->getVariablesDictionary();

  size_t numFolds = 7;
  double cvTrainResult = 0.0, cvTestResult = 0.0;
  for (size_t i = 0; i < numFolds; ++i)
  {
    VariableSetModelPtr model =
      independantClassificationVariableSetModel(createMaxentClassifier(labels));
      //optimisticClassificationVariableSetModel(createMaxentClassifier(labels))
      //iterativeClassificationVariableSetModel(createMaxentClassifier(labels), createMaxentClassifier(labels));
      //simulatedIterativeClassificationVariableSetModel(createMaxentClassifier(labels));

    std::cout << std::endl << std::endl << "FOLD " << (i+1) << " / " << numFolds << "...." << std::endl;
    
    ObjectContainerPtr trainingData = examples->invFold(i, numFolds);
    ObjectContainerPtr testingData = examples->fold(i, numFolds);
    
    ReferenceCountedObjectPtr<TestTrainingProgressCallback> callback
      = new TestTrainingProgressCallback(createLearningStoppingCriterion(), testingData);
    model->trainBatch(trainingData, callback);
    cvTrainResult += callback->trainAccuracy;
    cvTestResult += callback->testAccuracy;
  }
  
  std::cout << "Average Train Accuracy = " << cvTrainResult / numFolds << std::endl;
  std::cout << "Average Test Accuracy = " << cvTestResult / numFolds << std::endl;
 
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
