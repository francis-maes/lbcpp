/*-----------------------------------------.---------------------------------.
| Filename: ReductionInferenceStep.h   | Reduction from a problem        |
| Author  : Francis Maes                   |   to a simpler problem          |
| Started : 08/04/2010 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
# define LBCPP_REDUCTION_PREDICTION_PROBLEM_H_

# include "InferenceStep.h"

namespace lbcpp
{

class ReductionInferenceStep : public InferenceStep
{
public:
  ReductionInferenceStep(const String& name, InferenceStepPtr subProblem = InferenceStepPtr())
    : InferenceStep(name), subProblem(subProblem) {}
  
  virtual ObjectFunctionPtr createPreprocessFunction() const
    {return ObjectFunctionPtr();}

  virtual ObjectFunctionPtr createPostprocessFunction() const
    {return ObjectFunctionPtr();}

protected:
  virtual bool isModelUpToDate(const File& model, const Time& lastDataModificationTime) const
    {return subProblem->isModelUpToDate(model, lastDataModificationTime);}

  virtual ObjectFunctionPtr loadPredictor(const File& model) const
    {return subProblem->loadPredictor(model);}

  virtual ObjectFunctionPtr updatePredictor(const File& model, ObjectContainerPtr trainingData) const
  {
    return subProblem->updatePredictor(model, trainingData);
  }

protected:
  InferenceStepPtr subProblem;
};

}; /* namespace lbcpp */

#endif //!LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
