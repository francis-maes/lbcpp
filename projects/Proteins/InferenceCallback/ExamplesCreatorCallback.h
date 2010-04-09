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

namespace lbcpp
{

class ExamplesCreatorCallback : public InferenceCallback
{
public:
  ExamplesCreatorCallback() : enableExamplesCreation(true) {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode)
  {
    if (supervision && enableExamplesCreation)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(label);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
  }

protected:
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

  void trainAndFlushExamples()
  {
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      machine->trainStochastic(trainingData);
      std::cout << "ok." << std::endl;
    /*  ClassifierPtr classifier = machine.dynamicCast<Classifier>();
      if (classifier)
        std::cout << "Train accuracy: " << std::flush << classifier->evaluateAccuracy(trainingData) << std::endl;*/
    }
    examples.clear();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_EXAMPLES_CREATOR_H_
