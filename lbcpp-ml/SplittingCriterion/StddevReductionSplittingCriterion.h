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
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class StddevReductionSplittingCriterion : public SplittingCriterion
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
  {
    SplittingCriterion::configure(data, supervision, weights, indices);
    supervisions = getSupervisions().staticCast<DVector>();
  }

  virtual void update()
  {
    all.clear();
    positives.clear();
    negatives.clear();
    missings.clear();

    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double value = supervisions->get(it.getIndex());
      double weight = getWeight(it.getIndex());
      all.push(value, weight);
      switch (it.getRawBoolean())
      {
      case 0: negatives.push(value, weight); break;
      case 1: positives.push(value, weight); break;
      case 2: missings.push(value, weight); break;
      default: jassert(false);
      }
    }
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(upToDate);

    double value = supervisions->get(index);
    double weight = getWeight(index);
    negatives.push(value, -weight);
    positives.push(value, weight);
  }

  virtual double computeCriterion()
  {
    ensureIsUpToDate();

    double res = 0.0;
    if (positives.getCount())
      res += positives.getCount() * positives.getVariance();
    if (negatives.getCount())
      res += negatives.getCount() * negatives.getVariance();
    if (missings.getCount())
      res += missings.getCount() * missings.getVariance();
    jassert(predictions->size() == (size_t)(positives.getCount() + negatives.getCount() + missings.getCount()));
    if (predictions->size())
      res /= (double)predictions->size();
    double currentVariance = all.getVariance(); 
    jassert(res <= currentVariance);
    return (currentVariance - res) / currentVariance;
  }
  
  virtual ObjectPtr computeVote(const IndexSetPtr& indices)
  {
    ScalarVariableMean res;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
      res.push(supervisions->get(*it), getWeight(*it));
    return new Double(res.getMean());
  }

protected:
  DVectorPtr supervisions;

  ScalarVariableMeanAndVariance all;
  ScalarVariableMeanAndVariance positives;
  ScalarVariableMeanAndVariance negatives;
  ScalarVariableMeanAndVariance missings;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SPLITTING_CRITERION_STDDEV_REDUCTION_H_
