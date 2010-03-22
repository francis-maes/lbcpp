/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedClassifier.h      | Gradient based classifier       |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_

# include "StaticToDynamicGradientBasedMachine.h"

namespace lbcpp
{

class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  virtual void setLabels(StringDictionaryPtr labels)
    {architecture_.setOutputs(labels); GradientBasedClassifier::setLabels(labels);}
  
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return architecture_;}

  inline impl::MultiClassLogBinomialLoss<ClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<ClassificationExample>();}

private:
  impl::MultiLinearArchitecture architecture_;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_
