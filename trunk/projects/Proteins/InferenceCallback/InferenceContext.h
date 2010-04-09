/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.h             | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_H_
# define LBCPP_INFERENCE_CONTEXT_H_

# include "../InferenceStep/InferenceStep.h"

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

  bool isTopLevelInferenceCurrent() const
    {return stack.size() == 1;}

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

typedef ReferenceCountedObjectPtr<InferenceStack> InferenceStackPtr;

class InferenceCallback;
typedef ReferenceCountedObjectPtr<InferenceCallback> InferenceCallbackPtr;

class InferenceContext : public Object
{
public:
  typedef InferenceStep::ReturnCode ReturnCode;

  /*
  ** High level operations
  */
  virtual ReturnCode runWithSelfSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithUnsupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;

  /*
  ** Low level operations
  */
  virtual ObjectPtr runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runParallelInferences(ParallelInferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual FeatureGeneratorPtr runClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode) = 0;

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

protected:
  void callStartInferences(size_t count);
  void callFinishInferences();
  void callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
  void callClassification(InferenceStackPtr stack, ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
