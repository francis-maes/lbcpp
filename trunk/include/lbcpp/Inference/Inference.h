/*-----------------------------------------.---------------------------------.
| Filename: Inference.h                | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include "predeclarations.h"

namespace lbcpp
{

class Inference : public NameableObject
{
public:
  Inference(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual void accept(InferenceVisitorPtr visitor) = 0;

  // Used in SharedParallelInference before and after a block of many run() calls
  virtual void beginRunSession() {}
  virtual void endRunSession() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

  static InferencePtr createFromFile(const File& file)
    {return Object::createFromFileAndCast<Inference>(file);}

protected:
  friend class InferenceContext;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
