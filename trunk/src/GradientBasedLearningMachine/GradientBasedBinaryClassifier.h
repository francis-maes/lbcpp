/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedBinaryClassifier.h| Gradient based binary classifier|
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_BINARY_CLASSIFIER_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_BINARY_CLASSIFIER_H_

# include "StaticToDynamicGradientBasedMachine.h"

namespace lbcpp
{

template<class ExactType>
class StaticToDynamicGradientBasedBinaryClassifier
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().architecture());}
};

class LogisticRegressionClassifier : public StaticToDynamicGradientBasedBinaryClassifier<LogisticRegressionClassifier>
{
public:
  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::LogBinomialLoss<ClassificationExample> loss() const
    {return impl::logBinomialLoss<ClassificationExample>();}
};

class LinearSupportVectorMachine : public StaticToDynamicGradientBasedBinaryClassifier<LinearSupportVectorMachine>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::HingeLoss<ClassificationExample> loss() const
    {return impl::hingeLoss<ClassificationExample>();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_BINARY_CLASSIFIER_H_
