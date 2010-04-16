/*-----------------------------------------.---------------------------------.
| Filename: ExamplesCreatorCallback.h      | A callback that creates learning|
| Author  : Francis Maes                   |  examples                       |
| Started : 09/04/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_EXAMPLES_CREATOR_H_
# define LBCPP_INFERENCE_CALLBACK_EXAMPLES_CREATOR_H_

# include "InferenceCallback.h"
# include "../InferenceStep/ClassificationInferenceStep.h"

namespace lbcpp
{

// todo: file
class InferenceLearnerCallback : public Object
{
public:
  virtual InferenceContextPtr createContext() = 0;
  virtual ClassifierPtr createClassifier(FeatureDictionaryPtr labels) = 0;

  virtual void preLearningIterationCallback(size_t iterationNumber)
    {}

  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
    {return iterationNumber < 100;}

  virtual void preLearningPassCallback(const String& passName) {}
  virtual void postLearningPassCallback() {}
};

typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class ExamplesCreatorCallback : public InferenceCallback
{
public:
  ExamplesCreatorCallback(InferenceLearnerCallbackPtr learnerCallback)
    : learnerCallback(learnerCallback), enableExamplesCreation(true) {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (supervision && enableExamplesCreation)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(label);
      if (!classifier)
      {
        if (!supervision)
        {
          returnCode = InferenceStep::errorReturnCode;
          return;
        }
        FeatureGeneratorPtr correctOutput = supervision.dynamicCast<FeatureGenerator>();
        jassert(correctOutput);
        classifier = learnerCallback->createClassifier(correctOutput->getDictionary());
      }
      jassert(classifier);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
  }
  
  void trainStochasticIteration()
  {
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      machine->trainStochastic(trainingData);
      std::cout << "ok." << std::endl;
      /*GradientBasedClassifierPtr classifier = machine.dynamicCast<GradientBasedClassifier>();
      if (classifier)
      {
        std::cout << "Train accuracy: " << std::flush << classifier->evaluateAccuracy(trainingData) << " Num params = " << classifier->getParameters()->l0norm() << " Norm = " << classifier->getParameters()->l2norm() << std::endl;
      }*/
    }
  }

  void trainAndFlushExamples()
  {
    trainStochasticIteration();
    examples.clear();
  }

protected:
  InferenceLearnerCallbackPtr learnerCallback;
  bool enableExamplesCreation;

  typedef std::map<LearningMachinePtr, VectorObjectContainerPtr> ExamplesMap;
  ExamplesMap examples;

  void addExample(LearningMachinePtr learningMachine, ObjectPtr example)
  {
    VectorObjectContainerPtr& machineExamples = examples[learningMachine];
    if (!machineExamples)
      machineExamples = new VectorObjectContainer();
    machineExamples->append(example);
  }
};

typedef ReferenceCountedObjectPtr<ExamplesCreatorCallback> ExamplesCreatorCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_EXAMPLES_CREATOR_H_
