/*-----------------------------------------.---------------------------------.
| Filename: ReductionPredictionProblem.h   | Reduction from a problem        |
| Author  : Francis Maes                   |   to a simpler problem          |
| Started : 08/04/2010 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
# define LBCPP_REDUCTION_PREDICTION_PROBLEM_H_

# include "PredictionProblem.h"

namespace lbcpp
{

class ReductionPredictionProblem : public PredictionProblem
{
public:
  ReductionPredictionProblem(const String& name, PredictionProblemPtr subProblem = PredictionProblemPtr())
    : PredictionProblem(name), subProblem(subProblem) {}

  virtual bool isModelUpToDate(const File& model, const Time& lastDataModificationTime) const
    {return subProblem->isModelUpToDate(model, lastDataModificationTime);}

  virtual ObjectFunctionPtr loadPredictor(const File& model) const
    {return subProblem->loadPredictor(model);}

  virtual ObjectFunctionPtr updatePredictor(const File& model, ObjectContainerPtr trainingData) const
    {return subProblem->updatePredictor(model, trainingData);}

protected:
  PredictionProblemPtr subProblem;
};

}; /* namespace lbcpp */

#endif //!LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
