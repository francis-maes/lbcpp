/*-----------------------------------------.---------------------------------.
| Filename: RegressionInferenceStep.h      | Regression step                 |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_REGRESSION_H_
# define LBCPP_INFERENCE_STEP_REGRESSION_H_

# include "InferenceStep.h"
# include "../InferenceContext/InferenceVisitor.h"
# include "../InferenceContext/InferenceContext.h"

namespace lbcpp
{

// Input: FeatureGenerator
// Output: Scalar
// Supervision: Scalar
class RegressionInferenceStep : public InferenceStep
{
public:
  RegressionInferenceStep(const String& name)
    : InferenceStep(name) {}
  RegressionInferenceStep() {}

  virtual String toString() const
    {return getClassName();}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(RegressionInferenceStepPtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runRegression(RegressionInferenceStepPtr(this), input, supervision, returnCode);}

  RegressorPtr getRegressor() const
    {return regressor;}

  void setRegressor(RegressorPtr regressor)
    {this->regressor = regressor;}

  void createDotProductCache()
  {
    if (regressor)
    {
      GradientBasedRegressorPtr gbr = regressor.dynamicCast<GradientBasedRegressor>();
      jassert(gbr);
      gbr->createDotProductCache();
    }
  }

  void clearDotProductCache()
  {
    if (regressor)
    {
      GradientBasedRegressorPtr gbr = regressor.dynamicCast<GradientBasedRegressor>();
      jassert(gbr);
      gbr->clearDotProductCache();
    }
  }

protected:
  RegressorPtr regressor;

  virtual bool load(InputStream& istr)
    {return InferenceStep::load(istr) && lbcpp::read(istr, regressor);}

  virtual void save(OutputStream& ostr) const
    {InferenceStep::save(ostr); lbcpp::write(ostr, regressor);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_REGRESSION_H_
