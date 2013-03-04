/*-----------------------------------------.---------------------------------.
| Filename: InformationGainSplittingCriterion.h | Information Gain           |
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 13:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SPLITTING_CRITERION_INFORMATION_GAIN_H_
# define ML_SPLITTING_CRITERION_INFORMATION_GAIN_H_

# include <ml/SplittingCriterion.h>

namespace lbcpp
{

class InformationGainSplittingCriterion : public ClassificationSplittingCriterion
{
public:
  InformationGainSplittingCriterion(bool normalize = true)
    : normalize(normalize) {}
  
  virtual void getObjectiveRange(double& worst, double& best) const
  {
    // FIXME: 
    worst = 0.0;
    best = 1.0;
  }

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
  {
    ClassificationSplittingCriterion::configure(data, supervision, weights, indices);
    splitWeights = new DenseDoubleVector(3, 0.0); // prediction probabilities
    labelWeights = new DenseDoubleVector(labels, doubleClass); // label probabilities
    for (int i = 0; i < 3; ++i)
      labelConditionalProbabilities[i] = new DenseDoubleVector(labels, doubleClass); // label probabilities given that the predicted value is negative, positive or missing

    singleVoteVectors.resize(numLabels + 1);
    for (size_t i = 0; i < numLabels; ++i)
    {
      singleVoteVectors[i] = new DenseDoubleVector(labels, doubleClass);
      singleVoteVectors[i]->setValue(i, 1.0);
    }
    singleVoteVectors[numLabels] = new DenseDoubleVector(labels, doubleClass);
  }

  virtual void update()
  {
    splitWeights->multiplyByScalar(0.0);
    labelWeights->multiplyByScalar(0.0);
    for (size_t i = 0; i < 3; ++i)
      labelConditionalProbabilities[i]->multiplyByScalar(0.0);

    sumOfWeights = 0.0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t index = it.getIndex();
      double weight = getWeight(index);
      size_t supervision = (size_t)supervisions->get(index);
      unsigned char b = it.getRawBoolean();

      splitWeights->incrementValue((size_t)b, weight);
      labelWeights->incrementValue(supervision, weight);
      labelConditionalProbabilities[b]->incrementValue(supervision, weight);
      sumOfWeights += weight;
    }
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(upToDate);
    size_t supervision = (size_t)supervisions->get(index);
    double weight = getWeight(index);
    splitWeights->decrementValue(0, weight); // remove 'false' prediction
    labelConditionalProbabilities[0]->decrementValue(supervision, weight);
    splitWeights->incrementValue(1, weight); // add 'true' prediction
    labelConditionalProbabilities[1]->incrementValue(supervision, weight);
  }

  virtual double computeCriterion() const
  {
    const_cast<InformationGainSplittingCriterion* >(this)->ensureIsUpToDate();

    double currentEntropy = labelWeights->computeEntropy(sumOfWeights);
    double splitEntropy = splitWeights->computeEntropy(sumOfWeights);
    double expectedNextEntropy = 0.0;
    for (int i = 0; i < 3; ++i)
      expectedNextEntropy += (splitWeights->getValue(i) / sumOfWeights) * labelConditionalProbabilities[i]->computeEntropy(splitWeights->getValue(i));
    double informationGain = currentEntropy - expectedNextEntropy;
    if (normalize)
      return 2.0 * informationGain / (currentEntropy + splitEntropy);
    else
      return informationGain;
  }
  
  virtual ObjectPtr computeVote(const IndexSetPtr& indices)
  {
    if (indices->size() == 0)
      return singleVoteVectors[numLabels]; // (a vector of zeros)
    else if (indices->size() == 1)
    {
      // special case when the vote is all concentrated on a single label to spare some memory
      size_t label = (size_t)supervisions->get(*indices->begin());
      return singleVoteVectors[label]; // (a vector containing a single 1 on the label)
    }
    else
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(labels, doubleClass);
      double sumOfWeights = 0.0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
      {
        size_t index = *it;
        double weight = getWeight(index);
        res->incrementValue((size_t)supervisions->get(index), weight);
        sumOfWeights += weight;
      }
      if (sumOfWeights)
      {
        res->multiplyByScalar(1.0 / sumOfWeights);
        int argmax = res->getIndexOfMaximumValue();
        if (argmax >= 0 && fabs(res->getValue(argmax) - 1.0) < 1e-12)
          return singleVoteVectors[argmax]; // reuse an existing vector to spare memory
      }
      return res;
    }
  }

protected:
  friend class InformationGainSplittingCriterionClass;

  bool normalize;

  DenseDoubleVectorPtr splitWeights;
  DenseDoubleVectorPtr labelWeights;
  double sumOfWeights;
  DenseDoubleVectorPtr labelConditionalProbabilities[3];

  std::vector<DenseDoubleVectorPtr> singleVoteVectors;
};

}; /* namespace lbcpp */

#endif // !ML_SPLITTING_CRITERION_INFORMATION_GAIN_H_
