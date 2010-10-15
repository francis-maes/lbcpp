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
  const InferencePtr& getTopLevelInference() const
    {jassert(stack.size()); return stack[0];}

  const InferencePtr& getCurrentInference() const
    {jassert(stack.size()); return stack.back();}

  static InferencePtr nullInference;

  const InferencePtr& getParentInference() const
    {return stack.size() <= 1 ? nullInference : stack[stack.size() - 2];}

  InferencePtr getGrandParentInference() const
    {return stack.size() <= 2 ? nullInference : stack[stack.size() - 3];}

  InferencePtr getGrandGrandParentInference() const
    {return stack.size() <= 3 ? nullInference : stack[stack.size() - 4];}

  void push(const InferencePtr& inference)
    {jassert(inference); stack.push_back(inference);}

  void pop()
    {jassert(stack.size()); stack.pop_back();}

  size_t getDepth() const // 0 = not running, 1 = top level
    {return stack.size();}

  bool isInferenceRunning(const InferencePtr& inference, int* index = NULL)
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

  const InferencePtr& getInference(int index) const
    {return index >= 0 && index < (int)stack.size() ? stack[index] : nullInference;}

private:
  friend class InferenceStackClass;

  std::vector<InferencePtr> stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_STACK_H_
