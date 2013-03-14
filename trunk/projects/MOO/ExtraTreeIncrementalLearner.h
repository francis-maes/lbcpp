/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeIncrementalLearner.h  | ExtraTree Incremental Learner   |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_EXTRA_TREE_INCREMENTAL_LEARNER_H_
# define MOO_EXTRA_TREE_INCREMENTAL_LEARNER_H_

# include <ml/SplittingCriterion.h>
# include <ml/IncrementalLearner.h>

namespace lbcpp
{
  
class ScalarVectorExtraTreeNode
{
public:
  typedef std::vector<double> InputVector;
  typedef std::vector<double> Prediction;
  typedef std::pair<InputVector, Prediction> Sample;

  ScalarVectorExtraTreeNode(const InputVector& input, const Prediction& output)
    : testVariable((size_t)-1), testThreshold(0.0), left(NULL), right(NULL), samples(1, Sample(input, output)), prediction(output) {}

  ScalarVectorExtraTreeNode()
    : testVariable((size_t)-1), testThreshold(0.0), left(NULL), right(NULL) {}

  ~ScalarVectorExtraTreeNode()
  {
    if (left)
      delete left;
    if (right)
      delete right;
  }

  const ScalarVectorExtraTreeNode* findLeaf(const InputVector& input) const
    {return isLeaf() ? this : (input[testVariable] < testThreshold ? left : right)->findLeaf(input);}

  ScalarVectorExtraTreeNode* findLeaf(const InputVector& input)
    {return isLeaf() ? this : (input[testVariable] < testThreshold ? left : right)->findLeaf(input);}

  void addTrainingSample(ExecutionContext& context, const InputVector& input, const Prediction& output)
  {
    ScalarVectorExtraTreeNode* leaf = findLeaf(input);
    leaf->addSample(input, output);
    leaf->split(context);
  }

  const std::vector<double>& predict(const std::vector<double>& input) const
    {return findLeaf(input)->prediction;}

  bool isInternal() const
    {return left && right;}

  bool isLeaf() const
    {return !left && !right;}


private:
  // internal nodes
  size_t testVariable;
  double testThreshold;
  ScalarVectorExtraTreeNode* left;
  ScalarVectorExtraTreeNode* right;

  // leaf nodes
  std::vector<Sample> samples;
  Prediction prediction;

  void computePrediction()
  {
    jassert(samples.size());
    prediction.resize(samples[0].second.size());
    for (size_t i = 0; i < prediction.size(); ++i)
    {
      double& p = prediction[i];
      p = 0.0;
      for (size_t j = 0; j < samples.size(); ++j)
        p += samples[j].second[i];
      p /= (double)samples.size();
    }
  }

  void addSample(const InputVector& input, const Prediction& output)
    {samples.push_back(Sample(input, output));}

  void split(ExecutionContext& context)
  {
    jassert(isLeaf());

    size_t numAttributes = samples[0].first.size(); 
    testVariable = context.getRandomGenerator()->sampleSize(numAttributes);

    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;
    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first[testVariable];
      if (value < minValue)
        minValue = value;
      if (value > maxValue)
        maxValue = value;
    }
    if (minValue == maxValue)
      return;

    testThreshold = context.getRandomGenerator()->sampleDouble(minValue, maxValue);

    left = new ScalarVectorExtraTreeNode();
    right = new ScalarVectorExtraTreeNode();

    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first[testVariable];
      ScalarVectorExtraTreeNode* child = (value < testThreshold ? left : right);
      child->addSample(samples[i].first, samples[i].second);
    }

    left->computePrediction();
    right->computePrediction();

    samples.clear();
    prediction.clear();
  }
};

class ScalarVectorExtraTreeExpression : public Expression
{
public:
  ScalarVectorExtraTreeExpression() : root(NULL) {}
  virtual ~ScalarVectorExtraTreeExpression()
  {
    if (root)
      delete root;
  }

  void addTrainingSample(ExecutionContext& context, const std::vector<double>& input, const std::vector<double>& output)
  {
    if (root)
      root->addTrainingSample(context, input, output);
    else
      root = new ScalarVectorExtraTreeNode(input, output);
  }

  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
    {return root ? makeObject(root->predict(makeInput(inputs))) : ObjectPtr();}

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
  {
    if (!root)
      return DataVectorPtr();
    if (getType() == doubleClass)
    {
      DVectorPtr vector = new DVector(indices->size());
      size_t i = 0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
        vector->set(i++, root->predict(makeInput(data->getRow(*it)))[0]);
      return new DataVector(indices, vector);
    }
    else
    {
      OVectorPtr vector = new OVector(indices->size());
      size_t i = 0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
        vector->set(i++, compute(context, data->getRow(*it)));
      return new DataVector(indices, vector);
    }
  }

private:
  ScalarVectorExtraTreeNode* root;

  ObjectPtr makeObject(const std::vector<double>& prediction) const
  {
    if (prediction.size() == 1)
      return new Double(prediction[0]);
    else
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(getType());
      jassert(res->getNumValues() == prediction.size());
      for (size_t i = 0; i < prediction.size(); ++i)
        res->setValue(i, prediction[i]);
      return res;
    }
  }

  std::vector<double> makeInput(const std::vector<ObjectPtr>& sample) const
  {
    std::vector<double> res(sample.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = Double::get(sample[i]);
    return res;
  }
};

typedef ReferenceCountedObjectPtr<ScalarVectorExtraTreeExpression> ScalarVectorExtraTreeExpressionPtr;

class ScalarVectorExtraTreeIncrementalLearner : public IncrementalLearner
{
public:
  ScalarVectorExtraTreeIncrementalLearner(SplittingCriterionPtr splittingCriterion = SplittingCriterionPtr())
    : splittingCriterion(splittingCriterion) {}
  
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return new ScalarVectorExtraTreeExpression();}

  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expr) const
  {
    ScalarVectorExtraTreeExpressionPtr expression = expr.staticCast<ScalarVectorExtraTreeExpression>();

    std::vector<double> input(sample.size() - 1);
    for (size_t i = 0; i < input.size(); ++i)
      input[i] = Double::get(sample[i]);

    std::vector<double> output;
    ObjectPtr supervision = sample.back();
    if (supervision.isInstanceOf<Double>())
      output.resize(1, Double::get(supervision));
    else
      output = supervision.staticCast<DenseDoubleVector>()->getValues();
    expression->addTrainingSample(context, input, output);
  }

protected:
  friend class ScalarVectorExtraTreeIncrementalLearnerClass;

  SplittingCriterionPtr splittingCriterion;
};

class EnsembleIncrementalLearner : public IncrementalLearner
{
public:
  EnsembleIncrementalLearner(IncrementalLearnerPtr baseLearner, size_t ensembleSize)
    : baseLearner(baseLearner), ensembleSize(ensembleSize) {}
  EnsembleIncrementalLearner() : ensembleSize(0) {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
  {
    std::pair<AggregatorPtr, ClassPtr> aggregatorAndOutputType = Aggregator::create(supervisionType);
    AggregatorPtr aggregator = aggregatorAndOutputType.first;
    ClassPtr outputType = aggregatorAndOutputType.second;
    AggregatorExpressionPtr res = new AggregatorExpression(aggregator, outputType);
    res->reserveNodes(ensembleSize);
    for (size_t i = 0; i < ensembleSize; ++i)
      res->pushNode(baseLearner->createExpression(context, supervisionType));
    return res;
  }

  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expr) const
  {
    AggregatorExpressionPtr expression = expr.staticCast<AggregatorExpression>();
    for (size_t i = 0; i < expression->getNumSubNodes(); ++i)
      baseLearner->addTrainingSample(context, sample, expression->getSubNode(i));
  }

protected:
  friend class EnsembleIncrementalLearnerClass;

  IncrementalLearnerPtr baseLearner;
  size_t ensembleSize;
};

}; /* namespace lbcpp */

#endif // !MOO_SANDBOX_H_
