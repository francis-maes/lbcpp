/*-----------------------------------------.---------------------------------.
| Filename: ParameterizedInference.h       | Parameterized Inference         |
| Author  : Francis Maes                   |  base class                     |
| Started : 18/10/2010 11:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PARAMETERIZED_H_
# define LBCPP_INFERENCE_PARAMETERIZED_H_

# include "Inference.h"

namespace lbcpp
{

class ParameterizedInference : public Inference
{
public:
  ParameterizedInference(const String& name)
    : Inference(name) {}
  ParameterizedInference() {}

  virtual TypePtr getParametersType() const = 0;

  ObjectPtr getParameters() const;
  void setParameters(ObjectPtr parameters);

  virtual void clone(const ObjectPtr& target) const;

protected:
  virtual void parametersChangedCallback() {}

  friend class ParameterizedInferenceClass;

  juce::ReadWriteLock parametersLock;
  ObjectPtr parameters;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
