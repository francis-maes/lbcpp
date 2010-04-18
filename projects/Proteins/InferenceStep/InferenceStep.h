/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.h                | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include "../InferencePredeclarations.h"

namespace lbcpp
{

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual void accept(InferenceVisitorPtr visitor) = 0;
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

protected:
  friend class InferenceContext;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
