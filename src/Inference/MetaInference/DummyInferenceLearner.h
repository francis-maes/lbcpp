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

class DummyInferenceLearner : public InferenceLearner<Inference>
{
protected:
  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable run(InferenceContext* context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return Variable();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_DUMMY_LEARNER_H_
