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
# include <lbcpp/Inference/InferenceContext.h>
# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/FeatureGenerator/DenseVector.h>

namespace lbcpp
{

/*
** NumericalInference
*/
class NumericalInference : public Inference
{
public:
  NumericalInference(const String& name) : Inference(name) {}
  NumericalInference() {}

  virtual FeatureGeneratorPtr getExampleGradient(const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue) = 0;

  DenseVectorPtr getParameters() const
    {return parameters;}
 
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters; validateParametersChange();}

  virtual void validateParametersChange() {}

  virtual void clone(ObjectPtr target) const
  {
    Inference::clone(target);
    if (parameters)
    {
      NumericalInferencePtr targetInference = target.staticCast<NumericalInference>();
      targetInference->parameters = parameters->cloneAndCast<DenseVector>();
    }
  }

protected:
  friend class NumericalInferenceClass;

  DenseVectorPtr parameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_NUMERICAL_H_
