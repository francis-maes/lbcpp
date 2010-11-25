#ifndef GLOP_OFQI_INFERENCE_H_
# define GLOP_OFQI_INFERENCE_H_

# include <lbcpp/Inference/SequentialInference.h>

namespace lbcpp
{

class OFQIInference : public VectorSequentialInference
{
public:
  OFQIInference(ExecutionContext& context, size_t horizon, double discount, InferencePtr regressorModel)
    : VectorSequentialInference(T("OFQI")), discount(discount)
  {
    for (size_t i = 0; i < horizon; ++i)
      {
	InferencePtr regressor = regressorModel->cloneAndCast<Inference>(context);
	regressor->setName(T("OFQI - ") + String((int)i));
	appendInference(regressor);
      }
  }

  OFQIInference() {}

  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual TypePtr getOutputType() const
    {return doubleType;}  

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
  {
    Variable supervision;
    if (state->getSupervision().exists())
    {
      double value = state->getSupervision().getDouble();
      if (index > 0)
        value += state->getSubOutput().getDouble() * discount;  
      supervision = Variable(value);
    }
    state->setSubInference(subInferences[index], state->getInput(), supervision);
  }

private:
  friend class OFQIInferenceClass;

  double discount;
};

}; /* namespace lbcpp */

#endif // !GLOP_OFQI_INFERENCE_H_
