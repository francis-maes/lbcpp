/*-----------------------------------------.---------------------------------.
| Filename: IncrementalLearner.h           | Incremental Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_INCREMENTAL_LEARNER_H_
# define ML_INCREMENTAL_LEARNER_H_

# include <ml/Expression.h>

namespace lbcpp
{
  
class IncrementalLearner : public Object
{
public:
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const = 0;
    /** This method adds a training sample to the incremental learning algorithm.
   *  \param context The execution context
   *  \param sample The training sample, represented as a vector of length \f$n\f$. The first
   *                \f$n-1\f$ elements are considered the feature vector. The \f$n\f$th element
   *                is considered the target value. The target value can be een instance of
   *                Double or DoubleVector.
   *  \param expression The model which should be updated
   */
  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expression) const = 0;
};

typedef ReferenceCountedObjectPtr<IncrementalLearner> IncrementalLearnerPtr;

/**
 *  Abstract base class for Incremental Decision Tree Learners
 *  Subclasses should implement addSampleToTree
 */
class ScalarVectorTreeIncrementalLearner : public IncrementalLearner
{
public:
  virtual void addSampleToTree(ExecutionContext& context, TreeNodePtr root, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const = 0;
  
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return scalarVectorTreeNode();}

  /** This method adds a training sample to the incremental learning algorithm.
   *  \param context The execution context
   *  \param sample The training sample, represented as a vector of length \f$n\f$. The first
   *                \f$n-1\f$ elements are considered the feature vector. The \f$n\f$th element
   *                is considered the target value. The target value can be een instance of
   *                Double or DoubleVector.
   *  \param expression This should be a TreeNodePtr which represents the root of the tree
   */
  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expression) const
  {
    DenseDoubleVectorPtr input = new DenseDoubleVector(sample.size() - 1, 0.0);
    for (size_t i = 0; i < input->getNumValues(); ++i)
      input->setValue(i, Double::get(sample[i]));

    DenseDoubleVectorPtr output;;
    ObjectPtr supervision = sample.back();
    if (supervision.isInstanceOf<Double>())
      output = new DenseDoubleVector(1, Double::get(supervision));
    else
    {
      std::vector<double> vec = supervision.staticCast<DenseDoubleVector>()->getValues();
      output = new DenseDoubleVector(supervision.staticCast<DenseDoubleVector>()->getNumValues(), 0.0);
      size_t i = 0;
      for (std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it)
        output->setValue(i++,*it);
    }
    addSampleToTree(context, expression, input, output);
  }
};

extern IncrementalLearnerPtr pureRandomScalarVectorTreeIncrementalLearner();
extern IncrementalLearnerPtr ensembleIncrementalLearner(IncrementalLearnerPtr baseLearner, size_t ensembleSize);
extern IncrementalLearnerPtr perceptronIncrementalLearner(size_t numInitialTrainingSamples, double learningRate, double learningRateDecay);

}; /* namespace lbcpp */

#endif // !ML_INCREMENTAL_LEARNER_H_
