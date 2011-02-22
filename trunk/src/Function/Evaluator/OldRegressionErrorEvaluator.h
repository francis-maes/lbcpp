/*-----------------------------------------.---------------------------------.
| Filename: RegressionErrorEvaluator.h     | Regression Error Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OLD_EVALUATOR_REGRESSION_ERROR_H_
# define LBCPP_FUNCTION_OLD_EVALUATOR_REGRESSION_ERROR_H_

# include <lbcpp/Function/OldEvaluator.h>

namespace lbcpp
{

class OldDihedralRegressionErrorEvaluator : public OldRegressionErrorEvaluator
{
public:
  OldDihedralRegressionErrorEvaluator(const String& name) : OldRegressionErrorEvaluator(name) {}
  OldDihedralRegressionErrorEvaluator() {}

  virtual void addDelta(double delta)
    {OldRegressionErrorEvaluator::addDelta(normalizeAngle(delta));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OLD_EVALUATOR_REGRESSION_ERROR_H_
