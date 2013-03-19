/*-----------------------------------------.---------------------------------.
| Filename: VectorStddevReductionSplittingCriterion.h | Multi-dimensional    |
| Author  : Francis Maes                   | Standard Deviation Reduction    |
| Started : 09/01/2013 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SPLITTING_CRITERION_VECTOR_STDDEV_REDUCTION_H_
# define ML_SPLITTING_CRITERION_VECTOR_STDDEV_REDUCTION_H_

# include <ml/SplittingCriterion.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

class VectorStddevReductionSplittingCriterion : public SplittingCriterion
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
  {
    SplittingCriterion::configure(data, supervision, weights, indices);
    supervisions = getSupervisions().staticCast<OVector>();
    jassert(supervisions);
  }

  virtual void update()
  {
    infos.clear();
    infos.resize(DenseDoubleVector::getElementsEnumeration(supervisions->getElementsType())->getNumElements());

    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      DenseDoubleVectorPtr vector = supervisions->getAndCast<DenseDoubleVector>(it.getIndex());
      jassert(vector->getNumValues() == infos.size());
      double weight = getWeight(it.getIndex());

      for (size_t i = 0; i < infos.size(); ++i)
      {
        VarianceInfo& info = infos[i];
        double value = vector->getValue(i);
        info.all.push(value, weight);
        switch (it.getRawBoolean())
        {
        case 0: info.negatives.push(value, weight); break;
        case 1: info.positives.push(value, weight); break;
        case 2: info.missings.push(value, weight); break;
        default: jassert(false);
        }
      }
    }
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(upToDate);
    DenseDoubleVectorPtr vector = supervisions->getAndCast<DenseDoubleVector>(index);
    double weight = getWeight(index);
    for (size_t i = 0; i < infos.size(); ++i)
    {
      double value = vector->getValue(i);
      VarianceInfo& info = infos[i];
      info.negatives.push(value, -weight);
      info.positives.push(value, weight);
    }
  }

  virtual double computeCriterion() const
  {
    const_cast<VectorStddevReductionSplittingCriterion*>(this)->ensureIsUpToDate();

    double oldVariance = 0.0;
    double newVariance = 0.0;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      const VarianceInfo& info = infos[i];
      double var = 0.0;
      if (info.positives.getCount())
        var += info.positives.getCount() * info.positives.getVariance();
      if (info.negatives.getCount())
        var += info.negatives.getCount() * info.negatives.getVariance();
      if (info.missings.getCount())
        var += info.missings.getCount() * info.missings.getVariance();
      jassert(predictions->size() == (size_t)(info.positives.getCount() + info.negatives.getCount() + info.missings.getCount()));
      if (predictions->size())
        var /= (double)predictions->size();

      newVariance += var;
      oldVariance += info.all.getVariance();
    }

    jassert(newVariance <= oldVariance + 1e-9 && oldVariance > 1e-9);
    jassert(isNumberValid(oldVariance) && isNumberValid(newVariance));
    return (oldVariance - newVariance) / oldVariance;
  }
  
  virtual ObjectPtr computeVote(const IndexSetPtr& indices)
  {
    DenseDoubleVectorPtr res(new DenseDoubleVector(supervisions->getElementsType()));
    double sumOfWeights = 0.0;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    {
      DenseDoubleVectorPtr supervision = supervisions->getAndCast<DenseDoubleVector>(*it);
      double weight = getWeight(*it);
      sumOfWeights += weight;
      supervision->addWeightedTo(res, 0, weight);
    }
    if (sumOfWeights)
      res->multiplyByScalar(1.0 / sumOfWeights);
    return res;
  }

protected:
  OVectorPtr supervisions;

  struct VarianceInfo
  {
    ScalarVariableMeanAndVariance all;
    ScalarVariableMeanAndVariance positives;
    ScalarVariableMeanAndVariance negatives;
    ScalarVariableMeanAndVariance missings;
  };
  std::vector<VarianceInfo> infos;
};

}; /* namespace lbcpp */

#endif // !ML_SPLITTING_CRITERION_VECTOR_STDDEV_REDUCTION_H_
