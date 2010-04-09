/*-----------------------------------------.---------------------------------.
| Filename: GlobalSimulationLearningPolicy.h| A policy that performs global  |
| Author  : Francis Maes                   |  simulation to learn a policy   |
| Started : 09/04/2010 15:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_GLOBAL_SIMULATION_LEARNING_H_
# define LBCPP_INFERENCE_POLICY_GLOBAL_SIMULATION_LEARNING_H_

# include "ExamplesCreatorPolicy.h"

namespace lbcpp
{

class GlobalSimulationLearningPolicy : public ExamplesCreatorPolicy
{
public:
  GlobalSimulationLearningPolicy()
    : ExamplesCreatorPolicy(new DefaultInferencePolicy()) {}

  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
  {
    std::cout << "Creating training examples with " << examples->size() << " episodes..." << std::endl;
    this->examples.clear();
    return examples;
  }

  virtual void supervisedExampleSetPostCallback(InferenceStepPtr inference, ObjectContainerPtr , ReturnCode& returnCode)
  {
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      machine->trainStochastic(trainingData);
      ClassifierPtr classifier = machine.dynamicCast<Classifier>();
      if (classifier)
        std::cout << "Train accuracy: " << std::flush << classifier->evaluateAccuracy(trainingData) << std::endl;
    }
  }
};

typedef ReferenceCountedObjectPtr<GlobalSimulationLearningPolicy> GlobalSimulationLearningPolicyPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_POLICY_GLOBAL_SIMULATION_LEARNING_H_
