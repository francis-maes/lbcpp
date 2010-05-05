/*-----------------------------------------.---------------------------------.
| Filename: RegressionErrorEvaluator.h     | Regression Error Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_REGRESSION_ERROR_H_
# define LBCPP_EVALUATOR_REGRESSION_ERROR_H_

# include <lbcpp/Inference/Evaluator.h>

namespace lbcpp
{

class DihedralAngleRegressionErrorEvaluator : public RegressionErrorEvaluator
{
public:
  DihedralAngleRegressionErrorEvaluator(const String& name) : RegressionErrorEvaluator(name) {}

  virtual void addDelta(double delta)
    {RegressionErrorEvaluator::addDelta(normalizeAngle(delta));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_REGRESSION_ERROR_H_
