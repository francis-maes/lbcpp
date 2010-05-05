/*-----------------------------------------.---------------------------------.
| Filename: InferenceStack.h               | Inference Stack                 |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_STACK_H_
# define LBCPP_INFERENCE_CONTEXT_STACK_H_

# include "predeclarations.h"

namespace lbcpp
{

class InferenceStack : public Object
{
public:
  InferenceStepPtr getTopLevelInference() const
    {jassert(stack.size()); return stack[0];}

  InferenceStepPtr getCurrentInference() const
    {jassert(stack.size()); return stack.back();}

  InferenceStepPtr getParentInference() const
  {
    if (stack.size() <= 1)
      return InferenceStepPtr();
    return stack[stack.size() - 2];
  }

  void push(InferenceStepPtr inference)
    {stack.push_back(inference);}

  void pop()
    {jassert(stack.size()); stack.pop_back();}

  size_t getDepth() const // 0 = not running, 1 = top level
    {return stack.size();}

  bool isInferenceRunning(InferenceStepPtr inference, int* index = NULL)
  {
    for (size_t i = 0; i < stack.size(); ++i)
      if (stack[i] == inference)
      {
        if (index)
          *index = (int)i;
        return true;
      }
    if (index)
      *index = -1;
    return false;
  }

  InferenceStepPtr getInference(int index) const
    {return index >= 0 && index < (int)stack.size() ? stack[index] : InferenceStepPtr();}

private:
  std::vector<InferenceStepPtr> stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_STACK_H_
