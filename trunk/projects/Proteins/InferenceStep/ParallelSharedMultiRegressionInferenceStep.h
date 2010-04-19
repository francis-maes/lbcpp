/*-----------------------------------------.---------------------------------.
| Filename: ParallelSharedMultiRegressio..h| Regression with multiple outputs|
| Author  : Francis Maes                   |                                 |
| Started : 19/04/2010 17:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_PARALLEL_SHARED_MULTI_REGRESSION_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_PARALLEL_SHARED_MULTI_REGRESSION_INFERENCE_STEP_H_

# include "ParallelInferenceStep.h"
# include "RegressionInferenceStep.h"

namespace lbcpp
{

class ParallelSharedMultiRegressionInferenceStep : public SharedParallelInferenceStep
{
public:
  ParallelSharedMultiRegressionInferenceStep(const String& name, FeatureDictionaryPtr outputDictionary)
    : SharedParallelInferenceStep(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    return container->size();
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getInputFeatures(input, index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    DenseVectorPtr vector = supervision.dynamicCast<DenseVector>();
    jassert(vector);
    return new Scalar(vector->get(index));
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new DenseVector(outputDictionary, getNumSubInferences(input));}
  
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    DenseVectorPtr vector = output.dynamicCast<DenseVector>();
    jassert(vector);
    ScalarPtr scalar = subOutput.dynamicCast<Scalar>();
    jassert(scalar);
    vector->set(index, scalar->getValue());
  }

protected:
  FeatureDictionaryPtr outputDictionary;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_PARALLEL_SHARED_MULTI_REGRESSION_INFERENCE_STEP_H_
