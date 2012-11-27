/*-----------------------------------------.---------------------------------.
| Filename: StddevReductionSplittingCriterion.h |Standard Deviation Reduction|
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 12:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SPLITTING_CRITERION_STDDEV_REDUCTION_H_
# define LBCPP_ML_SPLITTING_CRITERION_STDDEV_REDUCTION_H_

# include <lbcpp-ml/SplittingCriterion.h>

namespace lbcpp
{

class StddevReductionSplittingCriterion : public SplittingCriterion
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const
  {
    // FIXME: 
    worst = 0.0;
    best = 1.0;
  }

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
  {
    SplittingCriterion::configure(data, supervision, weights, indices);
  }

  virtual void update()
  {
  }

  virtual void flipPrediction(size_t index)
  {
  }

  virtual double computeCriterion()
  {
    ensureIsUpToDate();
    return 0.0;
  }
  
  virtual ObjectPtr computeVote(const IndexSetPtr& indices)
  {
    return ObjectPtr();
  }

protected:
  friend class StddevReductionSplittingCriterionClass;

};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SPLITTING_CRITERION_STDDEV_REDUCTION_H_
