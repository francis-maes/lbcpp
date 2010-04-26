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

class VectorBasedInferenceHelper
{
public:
  virtual size_t getNumSubSteps() const
    {return subInferences.size();}

  virtual InferenceStepPtr getSubStep(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index];}

  void setSubStep(size_t index, InferenceStepPtr subStep)
    {jassert(index < subInferences.size()); subInferences[index] = subStep;}

  void appendStep(InferenceStepPtr inference)
    {subInferences.push_back(inference);}

  File getSubInferenceFile(size_t index, const File& directory) const;

  int findStepNumber(InferenceStepPtr step) const;

protected:
  std::vector<InferenceStepPtr> subInferences;

  bool saveSubInferencesToDirectory(const File& file) const;
  bool loadSubInferencesFromDirectory(const File& file);
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
