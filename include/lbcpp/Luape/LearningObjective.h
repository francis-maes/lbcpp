/*-----------------------------------------.---------------------------------.
| Filename: LearningObjective.h            | Learning Objectives             |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2012 20:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNING_OBJECTIVE_H_
# define LBCPP_LUAPE_LEARNING_OBJECTIVE_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include "LuapeCache.h"

namespace lbcpp
{

class LearningObjective : public Object
{
public:
  LearningObjective() : upToDate(false) {}

  virtual void initialize(const ExpressionDomainPtr& problem)
    {setSupervisions(problem->getTrainingSupervisions());}

  virtual void setSupervisions(const VectorPtr& supervisions) = 0;
  virtual void setWeights(const DenseDoubleVectorPtr& weights) = 0;
  virtual void setPredictions(const LuapeSampleVectorPtr& predictions) = 0;

  virtual void update() = 0;
  virtual void flipPrediction(size_t index) = 0; // flip from negative prediction to positive prediction
  virtual double computeObjective() = 0;

  // FunctionExpressionPtr computeBestStump(const IndexSetPtr& indices, const ExpressionPtr& numberNode, double& score) (== findBestThreshold)
  // ConstantExpressionPtr computeBestConstant(const IndexSetPtr& indices, double& score)  (== computeVote)

  virtual Variable computeVote(const IndexSetPtr& indices) = 0;

  // these three functions have side effects on the currently stored predictions
  double compute(const LuapeSampleVectorPtr& predictions);

  double computeObjectiveWithEventualStump(ExecutionContext& context, const ExpressionDomainPtr& problem, ExpressionPtr& weakNode, const IndexSetPtr& examples);
  double findBestThreshold(ExecutionContext& context, const ExpressionPtr& numberNode, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose = false);

  void ensureIsUpToDate();
  void invalidate()
    {upToDate = false;}

protected:
  bool upToDate;
};

typedef ReferenceCountedObjectPtr<LearningObjective> LearningObjectivePtr;

class SupervisedLearningObjective : public LearningObjective
{
public:
  virtual void setWeights(const DenseDoubleVectorPtr& weights)
    {this->weights = weights; invalidate();}

  const DenseDoubleVectorPtr& getWeights() const
    {return weights;}

  virtual void setPredictions(const LuapeSampleVectorPtr& predictions)
    {this->predictions = predictions; invalidate();}

protected:
  DenseDoubleVectorPtr weights;
  LuapeSampleVectorPtr predictions;

  double getWeight(size_t index) const
    {return weights ? weights->getValue(index) : 1.0;}
};

typedef ReferenceCountedObjectPtr<SupervisedLearningObjective> SupervisedLearningObjectivePtr;

class RegressionLearningObjective : public SupervisedLearningObjective
{
public:
  virtual void setSupervisions(const VectorPtr& supervisions);
  virtual Variable computeVote(const IndexSetPtr& indices);
  virtual void update();
  virtual void flipPrediction(size_t index);
  virtual double computeObjective();

  double getPositivesMean() const
    {return positives.getMean();}

  double getNegativesMean() const
    {return negatives.getMean();}
    
  double getMissingsMean() const
    {return missings.getMean();}

protected:
  DenseDoubleVectorPtr supervisions;

  ScalarVariableMeanAndVariance positives;
  ScalarVariableMeanAndVariance negatives;
  ScalarVariableMeanAndVariance missings;
};

typedef ReferenceCountedObjectPtr<RegressionLearningObjective> RegressionLearningObjectivePtr;

class BinaryClassificationLearningObjective : public SupervisedLearningObjective
{
public:
  BinaryClassificationLearningObjective();

  virtual void setSupervisions(const VectorPtr& supervisions);
  virtual Variable computeVote(const IndexSetPtr& indices);
  virtual void update();
  virtual void flipPrediction(size_t index);
  virtual double computeObjective();

  double getCorrectWeight() const
    {return correctWeight;}

  double getErrorWeight() const
    {return errorWeight;}

  double getMissingWeight() const
    {return missingWeight;}

protected:
  friend class BinaryClassificationLearningObjectiveClass;

  DenseDoubleVectorPtr supervisions;

  double correctWeight;
  double errorWeight;
  double missingWeight;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationLearningObjective> BinaryClassificationLearningObjectivePtr;

class ClassificationLearningObjective : public SupervisedLearningObjective
{
public:
  ClassificationLearningObjective() : numLabels(0) {}

  virtual void initialize(const ExpressionDomainPtr& problem);

protected:
  EnumerationPtr labels;
  size_t numLabels;
  ClassPtr doubleVectorClass;
};

typedef ReferenceCountedObjectPtr<ClassificationLearningObjective> ClassificationLearningObjectivePtr;

extern ClassificationLearningObjectivePtr discreteAdaBoostMHLearningObjective();
extern ClassificationLearningObjectivePtr realAdaBoostMHLearningObjective();

class InformationGainBinaryLearningObjective : public BinaryClassificationLearningObjective
{
public:
  InformationGainBinaryLearningObjective(bool normalize = true);

  virtual void initialize(const ExpressionDomainPtr& problem);
  virtual void setSupervisions(const VectorPtr& supervisions);
  virtual void update();
  virtual void flipPrediction(size_t index);
  virtual double computeObjective();
  virtual Variable computeVote(const IndexSetPtr& indices);

protected:
  friend class InformationGainBinaryLearningObjectiveClass;

  bool normalize;

  GenericVectorPtr supervisions;

  DenseDoubleVectorPtr splitWeights;
  DenseDoubleVectorPtr labelWeights;
  double sumOfWeights;
  DenseDoubleVectorPtr labelConditionalProbabilities[3];
};

class InformationGainLearningObjective : public ClassificationLearningObjective
{
public:
  InformationGainLearningObjective(bool normalize = true);

  virtual void initialize(const ExpressionDomainPtr& problem);
  virtual void setSupervisions(const VectorPtr& supervisions);
  virtual void update();
  virtual void flipPrediction(size_t index);
  virtual double computeObjective();
  virtual Variable computeVote(const IndexSetPtr& indices);

protected:
  friend class InformationGainLearningObjectiveClass;

  bool normalize;

  GenericVectorPtr supervisions;

  DenseDoubleVectorPtr splitWeights;
  DenseDoubleVectorPtr labelWeights;
  double sumOfWeights;
  DenseDoubleVectorPtr labelConditionalProbabilities[3];

  std::vector<DenseDoubleVectorPtr> singleVoteVectors;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNING_OBJECTIVE_H_
