/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorTreeExpression.h   | ScalarVectorTree Expression     |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               | Moved from project-moo by       |
`------------------------------------------/ Denny Verbeeck on 04/12/2013    |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SCALAR_VECTOR_TREE_EXPRESSION_H_
# define ML_SCALAR_VECTOR_TREE_EXPRESSION_H_

# include <ml/Expression.h>

namespace lbcpp
{

class ScalarVectorTreeNode : public TreeNode
{
public:
  typedef std::pair<DenseDoubleVectorPtr, DenseDoubleVectorPtr> Sample;
  
  ScalarVectorTreeNode(const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output)
    : TreeNode(-1, 0, denseDoubleVectorClass()), samples(1, Sample(input, output)), prediction(output) {}

  ScalarVectorTreeNode() : TreeNode(-1, 0, denseDoubleVectorClass()), prediction(new DenseDoubleVector(1, 0.0)) {}

  TreeNodePtr findLeaf(const ObjectPtr& input) const
    {return isLeaf() ? refCountedPointerFromThis<TreeNode>(this) : (input.staticCast<DenseDoubleVector>()->getValue(testVariable) < testThreshold ? left : right)->findLeaf(input);}

  const DenseDoubleVectorPtr& predict(const DenseDoubleVectorPtr& input) const
    {return findLeaf(input).staticCast<ScalarVectorTreeNode>()->prediction;}
    
  virtual ObjectPtr compute(ExecutionContext &context, const std::vector<ObjectPtr> &inputs) const
  {
    DenseDoubleVectorPtr inputVector = new DenseDoubleVector(inputs.size(), 0.0);
    size_t i = 0;
    for (std::vector<ObjectPtr>::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
      inputVector->setValue(i++, Double::get(*it));
    DenseDoubleVectorPtr pred = findLeaf(inputVector).staticCast<ScalarVectorTreeNode>()->prediction;
    if (pred->getNumValues() == 1)
      return new Double(pred->getValue(0));
    else
    {
      std::vector<double> predValues = std::vector<double>(pred->getValues());
      return new DenseDoubleVector(denseDoubleVectorClass(), predValues);
    }
  }

  virtual void addSample(const ObjectPtr& input, const ObjectPtr& output)
    {samples.push_back(Sample(input, output));}
  
  void computePrediction()
  {
    jassert(samples.size());
    prediction->resize(samples[0].second.staticCast<DenseDoubleVector>()->getNumValues());
    for (size_t i = 0; i < prediction->getNumValues(); ++i)
    {
      double& p = prediction->getValueReference(i);
      p = 0.0;
      for (size_t j = 0; j < samples.size(); ++j)
        p += samples[j].second.staticCast<DenseDoubleVector>()->getValue(i);
      p /= (double)samples.size();
    }
  }

  virtual void split(ExecutionContext& context, size_t testVariable, double testThreshold)
  {
    jassert(isLeaf());
    this->testVariable = testVariable;
    this->testThreshold = testThreshold;
  
    left = new ScalarVectorTreeNode();
    right = new ScalarVectorTreeNode();

    for (size_t i = 0; i < samples.size(); ++i)
    {
      double value = samples[i].first->getValue(testVariable);
      TreeNodePtr child = (value < testThreshold ? left : right);
      child->addSample(samples[i].first, samples[i].second);
    }

    left.staticCast<ScalarVectorTreeNode>()->computePrediction();
    right.staticCast<ScalarVectorTreeNode>()->computePrediction();

    samples.clear();
    prediction.clear();
  }

  virtual size_t getNumSamples() const
    {return samples.size();}
 
  virtual ObjectPtr getSampleInput(size_t index) const
    {jassert(index < samples.size()); return samples[index].first;}

  virtual ObjectPtr getSamplePrediction(size_t index) const
    {jassert(index < samples.size()); return samples[index].second;}

protected:
  friend class ScalarVectorTreeNodeClass;

  virtual DataVectorPtr computeSamples (ExecutionContext &context, const TablePtr &data, const IndexSetPtr &indices) const
  {
    DVectorPtr vector;
    ObjectPtr pred = compute(context, data->getRow(0));
    vector = new DVector(pred->getClass(), indices->size());
    size_t i = 0;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
      vector->setElement(i++, compute(context, data->getRow(*it)));
    return new DataVector(indices, vector);
  }

  std::vector<Sample> samples;
  DenseDoubleVectorPtr prediction;
};

typedef ReferenceCountedObjectPtr<ScalarVectorTreeNode> ScalarVectorTreeNodePtr;

/*
class ScalarVectorTreeExpression : public Expression
{
public:
  ScalarVectorTreeExpression(ClassPtr type = objectClass) 
    : Expression(type), root(NULL) {}

  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
    {return root ? root->compute(context, inputs) : ObjectPtr();}

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
  {
    if (!root)
      return DataVectorPtr();
    if (getType() == doubleClass)
    {
      DVectorPtr vector = new DVector(indices->size());
      size_t i = 0;
      for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
        vector->set(i++, root->compute(context, makeInput(data, *it)));
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
  ScalarVectorTreeNodePtr root;

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

  std::vector<ObjectPtr> makeInput(TablePtr data, size_t row) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(data->getNumColumns(), 0.0);
    for (size_t i = 0; i < res->getNumValues(); ++i)
      res->setValue(i, data->getData(i).staticCast<DVector>()->get(row));
    std::vector<ObjectPtr> result;
    result.push_back(res);
    return result;
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
*/
} /* namespace lbcpp */

#endif //!ML_SCALAR_VECTOR_TREE_EXPRESSION_H_
