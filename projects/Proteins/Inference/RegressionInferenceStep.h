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

protected:
  RegressorPtr regressor;

  virtual bool load(InputStream& istr)
    {return InferenceStep::load(istr) && lbcpp::read(istr, regressor);}

  virtual void save(OutputStream& ostr) const
    {InferenceStep::save(ostr); lbcpp::write(ostr, regressor);}
};


// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearScalarInferenceStep : public LearnableAtomicInferenceStep
{
public:
  LinearScalarInferenceStep(const String& name)
    : LearnableAtomicInferenceStep(name), dotProductCache(NULL) {}

  virtual ~LinearScalarInferenceStep()
    {clearDotProductCache();}

  void createDotProductCache()
  {
    clearDotProductCache();
    dotProductCache = new FeatureGenerator::DotProductCache();
  }

  void clearDotProductCache()
  {
    if (dotProductCache)
    {
      delete dotProductCache;
      dotProductCache = NULL;
    }
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    if (!parameters)
    {
      parameters = new DenseVector(features->getDictionary());
      return new Scalar(0.0);
    }
    return new Scalar(features->dotProduct(parameters, dotProductCache));
  }

  DenseVectorPtr getParameters() const
    {return parameters;}

private:
  DenseVectorPtr parameters;
  FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearScalarInferenceStep> LinearScalarInferenceStepPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_REGRESSION_H_
