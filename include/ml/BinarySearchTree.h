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
    leftStats(new ScalarVariableMeanAndVariance()), rightStats(new ScalarVariableMeanAndVariance()) {}

  virtual void insertValue(double attribute, double y)
  {
    if (value == DVector::missingValue)
      {value = attribute; return;}

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

  /** Find the best split point.
   *  \return a pair where the first element is the split point, and the second element is the split quality
   */
  std::pair<double, double> findBestSplitPoint()
  {
    // first calculate best of subtrees
    std::pair<double, double> bestLeft = std::make_pair(0.0, 0.0);
    std::pair<double, double> bestRight = std::make_pair(0.0, 0.0);
    std::pair<double, double> best;
    if (getLeft().exists()) bestLeft = left.staticCast<ExtendedBinarySearchTree>()->findBestSplitPoint(leftStats);
    if (getRight().exists()) bestRight = right.staticCast<ExtendedBinarySearchTree>()->findBestSplitPoint(rightStats);
    best = bestLeft.second >= bestRight.second ? bestLeft : bestRight;
    ScalarVariableMeanAndVariancePtr total = new ScalarVariableMeanAndVariance();
    total->push(leftStats->getSum(), leftStats->getCount());
    total->push(rightStats->getSum(), rightStats->getCount());
    double sdr = total->getStandardDeviation() - 
                 leftStats->getStandardDeviation() * leftStats->getCount() / total->getCount() -
                 rightStats->getStandardDeviation() * rightStats->getCount() / total->getCount();
    if (sdr > best.second)
      return std::make_pair(value, sdr);
    return best;
  }

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

  std::pair<double, double> findBestSplitPoint(ScalarVariableMeanAndVariancePtr statsParent)
  {
    if (isLeaf())
    {
      double sdr = statsParent->getStandardDeviation() - 
                   leftStats->getStandardDeviation() * leftStats->getCount() / statsParent->getCount() -
                   rightStats->getStandardDeviation() * rightStats->getCount() / statsParent->getCount();
      return std::make_pair(value, sdr);
    }
    std::pair<double, double> bestLeft = std::make_pair(0.0, 0.0);
    std::pair<double, double> bestRight = std::make_pair(0.0, 0.0);
    if (getLeft().exists()) bestLeft = left.staticCast<ExtendedBinarySearchTree>()->findBestSplitPoint(leftStats);
    if (getRight().exists()) bestRight = right.staticCast<ExtendedBinarySearchTree>()->findBestSplitPoint(rightStats);
    if (bestLeft.second >= bestRight.second) 
      return bestLeft;
    return bestRight;
  }
};

} /* namespace lbcpp */

#endif //!ML_DATA_BINARY_SEARCH_TREE_H_
