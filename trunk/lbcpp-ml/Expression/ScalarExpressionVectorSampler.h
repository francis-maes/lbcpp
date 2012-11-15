/*-----------------------------------------.---------------------------------.
| Filename: ScalarExpressionVectorSampler.h| Scalar Expression Vector Sampler|
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 16:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_SCALAR_VECTOR_SAMPLER_H_
# define LBCPP_ML_EXPRESSION_SCALAR_VECTOR_SAMPLER_H_

# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class ScalarExpressionVectorSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionDomainPtr expressionDomain = domain.staticCast<VectorDomain>()->getElementsDomain().staticCast<ExpressionDomain>();
    expressions = vector(expressionClass, 0);
    for (size_t i = 0; i < expressionDomain->getNumInputs(); ++i)
    {
      VariableExpressionPtr input = expressionDomain->getInput(i);
      if (input->getType()->isConvertibleToDouble())
        expressions->append(input);
    }
  }

  virtual ObjectPtr sample(ExecutionContext& context) const 
    {return expressions;}

private:
  OVectorPtr expressions;
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_EXPRESSION_SCALAR_VECTOR_SAMPLER_H_
