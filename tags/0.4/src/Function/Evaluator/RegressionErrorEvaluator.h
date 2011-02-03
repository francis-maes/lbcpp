/*-----------------------------------------.---------------------------------.
| Filename: RegressionErrorEvaluator.h     | Regression Error Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_
# define LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class DihedralRegressionErrorEvaluator : public RegressionErrorEvaluator
{
public:
  DihedralRegressionErrorEvaluator(const String& name) : RegressionErrorEvaluator(name) {}
  DihedralRegressionErrorEvaluator() {}

  virtual void addDelta(double delta)
    {RegressionErrorEvaluator::addDelta(normalizeAngle(delta));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_
