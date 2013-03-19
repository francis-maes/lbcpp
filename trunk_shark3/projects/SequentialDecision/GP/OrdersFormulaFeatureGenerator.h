/*-----------------------------------------.---------------------------------.
| Filename: OrdersFormulaFeatureGenerator.h| Orders Formula Features         |
| Author  : Francis Maes                   |                                 |
| Started : 25/09/2011 20:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GP_ORDERS_FORMULA_FEATURE_GENERATOR_H_
# define LBCPP_SEQUENTIAL_DECISION_GP_ORDERS_FORMULA_FEATURE_GENERATOR_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include "GPExpression.h"

namespace lbcpp
{

class OrdersFormulaFeatureGenerator : public FeatureGenerator
{
public:
  OrdersFormulaFeatureGenerator(const std::vector< std::vector<double> >& inputs)
    : inputs(inputs) {}
  OrdersFormulaFeatureGenerator() {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return gpExpressionClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return positiveIntegerEnumerationEnumeration;}

  virtual void computeFeatures(const Variable* in, FeatureGeneratorCallback& callback) const
  {
    const GPExpressionPtr& formula = in[0].getObjectAndCast<GPExpression>();

    for (size_t i = 0; i < inputs.size(); ++i)
    {
      const std::vector<double>& variables = inputs[i];
      double value1 = formula->compute(&variables[0]);
      double value2 = formula->compute(&variables[0] + variables.size() / 2);
      if (value1 < value2)
        callback.sense(2 * i, 1.0);
      else if (value1 > value2)
        callback.sense(2 * i + 1, 1.0);
    }
  }

protected:
  std::vector< std::vector<double> > inputs;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_ORDERS_FORMULA_FEATURE_GENERATOR_H_
