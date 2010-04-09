/*-----------------------------------------.---------------------------------.
| Filename: GlobalSimulationLearningCal...h| A callback that performs        |
| Author  : Francis Maes                   |  learning on everything         |
| Started : 09/04/2010 19:49               |  simultaneously                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_

# include "ExamplesCreatorCallback.h"

namespace lbcpp
{

class GlobalSimulationLearningCallback : public ExamplesCreatorCallback
{
public:
  virtual void startInferencesCallback(size_t count)
    {std::cout << "Creating training examples with " << count << " episodes..." << std::endl;}

  virtual void finishInferencesCallback()
    {trainAndFlushExamples();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_GLOBAL_SIMULATION_LEARNING_H_
