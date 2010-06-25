/*-----------------------------------------.---------------------------------.
| Filename: ParameterizedInference.h       | Parameterized Inference         |
| Author  : Francis Maes                   |   base class                    |
| Started : 27/05/2010 21:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PARAMETERIZED_H_
# define LBCPP_INFERENCE_PARAMETERIZED_H_

# include "Inference.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../FeatureGenerator/FeatureGenerator.h"
# include "../FeatureGenerator/DenseVector.h"
# include "../Object/ObjectPair.h"

namespace lbcpp
{

/*
** ParameterizedInference
*/
class ParameterizedInference : public Inference
{
public:
  ParameterizedInference(const String& name) : Inference(name) {}
  ParameterizedInference() {}

  virtual FeatureGeneratorPtr getExampleGradient(const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue) = 0;

  DenseVectorPtr getParameters() const
    {return parameters;}
 
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters; validateParametersChange();}

  virtual ObjectPtr clone() const;

  virtual void validateParametersChange() {}

protected:
  DenseVectorPtr parameters;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARAMETERIZED_H_
