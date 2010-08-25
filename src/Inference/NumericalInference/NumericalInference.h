/*-----------------------------------------.---------------------------------.
| Filename: NumericalInference.h           | Numerical Inference             |
| Author  : Francis Maes                   |   base class                    |
| Started : 27/05/2010 21:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_NUMERICAL_H_
# define LBCPP_INFERENCE_NUMERICAL_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Function/Perception.h>
# include <lbcpp/Function/PerceptionMaths.h>

namespace lbcpp
{

/*
** NumericalInference
*/
class NumericalInference : public Inference
{
public:
  NumericalInference(const String& name, PerceptionPtr perception)
    : Inference(name), perception(perception) {}
  NumericalInference() {}

  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  TypePtr getPerceptionOutputType() const
    {return perception->getOutputType();}

  virtual void computeAndAddGradient(ObjectPtr& target, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue) = 0;

  ObjectPtr getParameters() const
    {return parameters;}
 
  ObjectPtr& getParameters()
    {return parameters;}

  PerceptionPtr getPerception() const
    {return perception;}

  void setParameters(ObjectPtr parameters)
    {this->parameters = parameters; validateParametersChange();}

  virtual void validateParametersChange() {}

protected:
  friend class NumericalInferenceClass;

  PerceptionPtr perception;
  ObjectPtr parameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_NUMERICAL_H_
