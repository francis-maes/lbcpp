/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorTreeExpression.h   | ScalarVector Decision Trees     |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 17:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_SCALAR_VECTOR_TREE_EXPRESSION_H_
# define MOO_SCALAR_VECTOR_TREE_EXPRESSION_H_

# include <ml/Expression.h>

namespace lbcpp
{
  
class ScalarVectorTreeNode
{
public:
  typedef std::vector<double> InputVector;
  typedef std::vector<double> Prediction;
  typedef std::pair<InputVector, Prediction> Sample;

  ScalarVectorTreeNode(const InputVector& input, const Prediction& output)
    : testVariable((size_t)-1), testThreshold(0.0), left(NULL), right(NULL), samples(1, Sample(input, output)), prediction(output) {}

  ScalarVectorTreeNode()
    : testVariable((size_t)-1), testThreshold(0.0), left(NULL), right(NULL) {}

  ~ScalarVectorTreeNode()
  {
    if (left)
      delete left;
    if (right)
      delete right;
  }

  const ScalarVectorTreeNode* findLeaf(const InputVector& input) const
    {return isLeaf() ? this : (input[testVariable] < testThreshold ? left : right)->findLeaf(input);}

  ScalarVectorTreeNode* findLeaf(const InputVector& input)
    {return isLeaf() ? this : (input[testVariable] < testThreshold ? left : right)->findLeaf(input);}

  const std::vector<double>& predict(const std::vector<double>& input) const
    {return findLeaf(input)->prediction;}

  bool isInternal() const
    {return left && right;}

  void addSample(const InputVector& input, const Prediction& output)
    {samples.push_back(Sample(input, output));}
  
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

  void split(ExecutionContext& context, size_t testVariable, double testThreshold)
  {
    jassert(isLeaf());
    this->testVariable = testVariable;
    this->testThreshold = testThreshold;
  
    left = new ScalarVectorTreeNode();
    right = new ScalarVectorTreeNode();

    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first[testVariable];
      ScalarVectorTreeNode* child = (value < testThreshold ? left : right);
      child->addSample(samples[i].first, samples[i].second);
    }

    left->computePrediction();
    right->computePrediction();

    samples.clear();
    prediction.clear();
  }

  // Leafs
  bool isLeaf() const
    {return !left && !right;}  

  size_t getNumSamples() const
    {return samples.size();}

  const InputVector& getSampleInput(size_t index) const
    {jassert(index < samples.size()); return samples[index].first;}

  const Prediction& getSamplePrediction(size_t index) const
    {jassert(index < samples.size()); return samples[index].second;}

private:
  // internal nodes
  size_t testVariable;
  double testThreshold;
  ScalarVectorTreeNode* left;
  ScalarVectorTreeNode* right;

  // leaf nodes
  std::vector<Sample> samples;
  Prediction prediction;
};

class ScalarVectorTreeExpression : public Expression
{
public:
  ScalarVectorTreeExpression() : root(NULL) {}
  virtual ~ScalarVectorTreeExpression()
  {
    if (root)
      delete root;
  }

  typedef std::vector<double> InputVector;
  typedef std::vector<double> Prediction;

  bool hasRoot() const
    {return root != NULL;}

  ScalarVectorTreeNode* getRoot() const
    {return root;}

  void createRoot(const InputVector& input, const Prediction& output)
  {
    jassert(!root);
    root = new ScalarVectorTreeNode(input, output);
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
  ScalarVectorTreeNode* root;

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

typedef ReferenceCountedObjectPtr<ScalarVectorTreeExpression> ScalarVectorTreeExpressionPtr;

}; /* namespace lbcpp */

#endif // !MOO_SCALAR_VECTOR_TREE_EXPRESSION_H_
