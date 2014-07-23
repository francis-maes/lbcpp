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
# include <ml/Solver.h>

namespace lbcpp
{
  
class IncrementalLearner : public Object
{
public:
  IncrementalLearner() : verbosity((SolverVerbosity) 0) {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const = 0;
  /** This method adds a training sample to the incremental learning algorithm. This method is the most general case,
   *  if your incremental learner can deal with non-numerical data, you should probably override this method in child
   *  classes. By default this method transforms the first \f$n-1\f$ elements to a DenseDoubleVector representing the
   *  input, and the last element to a DenseDoubleVector representing the output.
   *  \param context The execution context
   *  \param sample The training sample, represented as a vector of length \f$n\f$. The first
   *                \f$n-1\f$ elements are considered the feature vector. The \f$n\f$th element
   *                is considered the target value.
   *  \param expression The model which should be updated
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
    addTrainingSample(context, expression, input, output);
  }
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const = 0;

  virtual void initialiseLearnerStatistics(ExecutionContext& context, ExpressionPtr model, ObjectPtr data) const {}

  void setVerbosity(SolverVerbosity verbosity)
    {this->verbosity = verbosity;}

  SolverVerbosity getVerbosity()
    {return verbosity;}

protected:
  SolverVerbosity verbosity;
};

typedef ReferenceCountedObjectPtr<IncrementalLearner> IncrementalLearnerPtr;

/** Base class for incremental splitting criteria 
 *  Subclasses should implement findBestSplit
 */
class IncrementalSplittingCriterion : public Object
{
public:
  struct Split
  {
    Split() : attribute(0), value(0.0), quality(0.0) {}
    Split(size_t attribute, double value, double quality) : attribute(attribute), value(value), quality(quality) {}
    size_t attribute;
    double value;
    double quality;
    double rstd;//residual standard deviation
    double rssCombined; // residual sum of squares of the parent node
    double rssLeft; // residual sum of squares of the left split
    double rssRight; // residual sum of squares of the right split
    size_t N; // number of samples seen to make the split
  };

  /** Find the best split point for this leaf.
   *  \return A Split struct containing the attribute index, split value and split quality.
   *          If no suitable Split was found, set quality to DVector::missingValue to indicate this.
   */
  virtual Split findBestSplit(ExecutionContext& context, TreeNodePtr leaf) const = 0;
  virtual double splitQuality(ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const = 0;
};

typedef ReferenceCountedObjectPtr<IncrementalSplittingCriterion> IncrementalSplittingCriterionPtr;

extern IncrementalLearnerPtr pureRandomScalarVectorTreeIncrementalLearner();
extern IncrementalLearnerPtr ensembleIncrementalLearner(IncrementalLearnerPtr baseLearner, size_t ensembleSize);
extern IncrementalLearnerPtr perceptronIncrementalLearner(size_t numInitialTrainingSamples, double learningRate, double learningRateDecay);
extern IncrementalLearnerPtr hoeffdingTreeIncrementalLearner(IncrementalSplittingCriterionPtr splittingCriterion, IncrementalLearnerPtr perceptronLearner, size_t chunkSize = 50);
extern IncrementalLearnerPtr simpleLinearRegressionIncrementalLearner();
extern IncrementalLearnerPtr linearLeastSquaresRegressionIncrementalLearner();

extern IncrementalSplittingCriterionPtr hoeffdingBoundMauveIncrementalSplittingCriterion(double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundExtendedMauveIncrementalSplittingCriterion(double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundStdDevReductionIncrementalSplittingCriterion(double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundTotalMauveIncrementalSplittingCriterion(double delta, double threshold);
/*extern IncrementalSplittingCriterionPtr hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(double delta, double threshold);
extern IncrementalSplittingCriterionPtr mauveIncrementalSplittingCriterion(double delta, double threshold, double maxCoefficientOfDetermination);
extern IncrementalSplittingCriterionPtr quandtAndrewsIncrementalSplittingCriterion(size_t numVariables, double threshold);*/
extern IncrementalSplittingCriterionPtr nullIncrementalSplittingCriterion();


class IncrementalLearnerStatistics : public Object
{
public:
  IncrementalLearnerStatistics() : examplesSeen(0) {}

  size_t getExamplesSeen()
    {return examplesSeen;}

  void incrementExamplesSeen(size_t amount = 1)
    {examplesSeen += amount;}

protected:
  friend class IncrementalLearnerStatisticsClass;

  size_t examplesSeen;
};

extern IncrementalLearnerStatisticsPtr hoeffdingTreeIncrementalLearnerStatistics(size_t numAttributes = 0);
extern IncrementalLearnerStatisticsPtr hoeffdingTreeIncrementalLearnerStatistics(ExecutionContext& context, IncrementalLearnerStatisticsPtr parentStats, size_t attribute, double splitValue, bool leftSide);

}; /* namespace lbcpp */

#endif // !ML_INCREMENTAL_LEARNER_H_
