/*-----------------------------------------.---------------------------------.
| Filename: BinarySearchTree.h             | Binary Search Tree              |
| Author  : Denny Verbeeck                 |                                 |
| Started : 12/03/2014 16:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_DATA_BINARY_SEARCH_TREE_H_
# define ML_DATA_BINARY_SEARCH_TREE_H_

# include <ml/predeclarations.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

/**
 * Base class for binary search trees
 */
class BinarySearchTree : public Object
{
public:
  BinarySearchTree(double value = DVector::missingValue) : value(value), left(BinarySearchTreePtr()), right(BinarySearchTreePtr()) {}
  
  BinarySearchTreePtr getLeft() const
    {return left;}

  BinarySearchTreePtr getRight() const
    {return right;}

  bool isLeaf() const
    {return !left.exists() && !right.exists();}

  double getValue() const
    {return value;}

  virtual void insertValue(double attribute, double y) = 0;

protected:
  friend class BinarySearchTreeClass;

  double value;
  BinarySearchTreePtr left;
  BinarySearchTreePtr right;
};

class ExtendedBinarySearchTree : public BinarySearchTree
{
public:
  ExtendedBinarySearchTree(double value = DVector::missingValue) : BinarySearchTree(value),
    leftStats(new ScalarVariableMeanAndVariance()), rightStats(new ScalarVariableMeanAndVariance()),
    correlation(new PearsonCorrelationCoefficient()) {}

  virtual void insertValue(double attribute, double y)
  {
    if (value == DVector::missingValue)
    {
      // This node does not have a value yet
      value = attribute;
      leftStats->push(y); 
    }
    else 
      if (attribute <= value)
      {
        leftStats->push(y);
        if (!left.exists())
          left = new ExtendedBinarySearchTree(attribute);
        else
          left->insertValue(attribute, y);
      }
      else
      {
        rightStats->push(y);
        if (!right.exists())
          right = new ExtendedBinarySearchTree(attribute);
        else
          right->insertValue(attribute, y);
      }
  }

  ScalarVariableMeanAndVariancePtr getLeftStats()
    {return leftStats;}

  ScalarVariableMeanAndVariancePtr getRightStats()
    {return rightStats;}

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    ExtendedBinarySearchTreePtr result = new ExtendedBinarySearchTree();
    result->value = value;
    result->leftStats = leftStats->clone(context);
    result->rightStats = rightStats->clone(context);
    if (left.exists())
      result->left = left->clone(context);
    if (right.exists())
      result->right = right->clone(context);
    return result;
  }

protected:
  friend class ExtendedBinarySearchTreeClass;

  ScalarVariableMeanAndVariancePtr leftStats;
  ScalarVariableMeanAndVariancePtr rightStats;
  CorrelationCoefficientPtr correlation;
};

} /* namespace lbcpp */

#endif //!ML_DATA_BINARY_SEARCH_TREE_H_
