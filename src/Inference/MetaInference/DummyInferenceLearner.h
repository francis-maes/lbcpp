/*-----------------------------------------.---------------------------------.
| Filename: DummyInferenceLearner.h        | A batch learner that            |
| Author  : Francis Maes                   |  does nothing                   |
| Started : 14/07/2010 13:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_DUMMY_LEARNER_H_
# define LBCPP_INFERENCE_META_DUMMY_LEARNER_H_

# include <lbcpp/Inference/Inference.h>

namespace lbcpp
{

class DummyInferenceLearner : public Inference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(inferenceClass(), containerClass(pairType(anyType(), anyType())));}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

protected:
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return Variable();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_DUMMY_LEARNER_H_
