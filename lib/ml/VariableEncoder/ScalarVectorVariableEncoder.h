/*------------------------------------------.---------------------------------.
 | Filename: ScalarVectorVariableEncoder.h  | Scalar Vector Variable Encoder  |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 22/02/2013 17:15               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SCALAR_VECTOR_VARIABLE_ENCODER_H_
# define ML_SCALAR_VECTOR_VARIABLE_ENCODER_H_

# include <ml/VariableEncoder.h>
# include <ml/ExpressionDomain.h>

namespace lbcpp
{

class ScalarVectorVariableEncoder : public VariableEncoder
{
public:
  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res)
  {
    ScalarVectorDomainPtr continuousDomain = domain.staticCast<ScalarVectorDomain>();
    for (size_t i = 0; i < continuousDomain->getNumDimensions(); ++i)
      res->addInput(doubleClass, "x" + string((int)i + 1));
  }
  
  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr object, std::vector<ObjectPtr>& res)
  {
    DenseDoubleVectorPtr vector = object.staticCast<DenseDoubleVector>();
    for (size_t i = 0; i < vector->getNumValues(); ++i)
      res.push_back(new Double(vector->getValue(i)));
  }
};

}; /* namespace lbcpp */

#endif // !ML_SCALAR_VECTOR_VARIABLE_ENCODER_H_
