/*-----------------------------------------.---------------------------------.
| Filename: InferenceWorkUnit.h            | Inference Work Unit             |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 13:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_WORK_UNIT_H_
# define LBCPP_INFERENCE_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Inference/Inference.h>

namespace lbcpp
{

class InferenceWorkUnit : public WorkUnit
{
public:
  InferenceWorkUnit(const String& name, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output)
    : WorkUnit(name), inference(inference), input(input), supervision(supervision), output(output)
    {}

  InferenceWorkUnit()
    : output(*(Variable* )0)
    {}

  virtual String toShortString() const
    {return getName();}

  virtual String toString() const
    {return getName();}

  virtual bool run(ExecutionContext& context)
    {return inference->run(context, input, supervision, output);}

protected:
  friend class InferenceWorkUnitClass;

  InferencePtr inference;
  Variable input;
  Variable supervision;
  Variable& output;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_WORK_UNIT_H_
