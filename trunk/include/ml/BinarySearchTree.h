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

  virtual void insertValue(double attribute, const DenseDoubleVectorPtr& data, double y) = 0;

  // finds the node with a value that equals the given value
  // if no such node exists, NULL is returned
  virtual BinarySearchTreePtr getNode(double val) const
  {
    if(value == val)
      return refCountedPointerFromThis(this);
    else if(val < value && left.exists())
      return left->getNode(val);
    else if(val > value && right.exists())
      return right->getNode(val);
    else
      return NULL;
  }

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
    leftCorrelation(new MultiVariateRegressionStatistics()), rightCorrelation(new MultiVariateRegressionStatistics()) {}

  virtual void insertValue(double attribute, const DenseDoubleVectorPtr& data, double y)
  {
    if (value == DVector::missingValue)
    {
      // This node does not have a value yet
      value = attribute;
      leftStats->push(y);
      leftCorrelation->push(data, y);
    }
    else
    {
      if (attribute == value)
      {
        leftStats->push(y);
        leftCorrelation->push(data, y);
      }
      else if (attribute < value)
      {
        leftStats->push(y);
    		leftCorrelation->push(data, y);
        if (!left.exists())
        {
          left = new ExtendedBinarySearchTree();
        }
        left->insertValue(attribute, data, y);
      }
      else
      {
        rightStats->push(y);
        rightCorrelation->push(data, y);
        if (!right.exists())
        {
          right = new ExtendedBinarySearchTree();
        }
        right->insertValue(attribute, data, y);
      }
    }
  }

  ScalarVariableMeanAndVariancePtr getLeftStats()
    {return leftStats;}

  ScalarVariableMeanAndVariancePtr getRightStats()
    {return rightStats;}

  MultiVariateRegressionStatisticsPtr getLeftCorrelation()
    {return leftCorrelation;}

  MultiVariateRegressionStatisticsPtr getRightCorrelation()
    {return rightCorrelation;}

    /* Calculate total regression statistics for a split
   * \param splitValue The split value
   * \return a pair of PearsonCorrelationCoefficientPtrs where the first and second values represent statistics
   *         for left and right of the split respectively
   */

  std::pair<MultiVariateRegressionStatisticsPtr, MultiVariateRegressionStatisticsPtr> getStatsForSplit(double splitValue)
  {
    MultiVariateRegressionStatisticsPtr leftStats, rightStats;
    leftStats = new MultiVariateRegressionStatistics();
    rightStats = new MultiVariateRegressionStatistics();
    if (splitValue < value)
    {
      std::pair<MultiVariateRegressionStatisticsPtr, MultiVariateRegressionStatisticsPtr> stats = left.staticCast<ExtendedBinarySearchTree>()->getStatsForSplit(splitValue);
      leftStats = stats.first;
      rightStats = stats.second;
      rightStats->update(*rightCorrelation);
      return std::make_pair(leftStats, rightStats);
    }
    else if (splitValue > value)
    {
      std::pair<MultiVariateRegressionStatisticsPtr, MultiVariateRegressionStatisticsPtr> stats = right.staticCast<ExtendedBinarySearchTree>()->getStatsForSplit(splitValue);
      leftStats = stats.first;
      rightStats = stats.second;
      leftStats->update(*leftCorrelation);
      return std::make_pair(leftStats, rightStats);
    }
    return std::make_pair(new MultiVariateRegressionStatistics(*leftCorrelation), new MultiVariateRegressionStatistics(*rightCorrelation));
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    ExtendedBinarySearchTreePtr result = new ExtendedBinarySearchTree();
    result->value = value;
    result->leftStats = leftStats->clone(context);
    result->rightStats = rightStats->clone(context);
    result->leftCorrelation = leftCorrelation->clone(context);
    result->rightCorrelation = rightCorrelation->clone(context);
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
  MultiVariateRegressionStatisticsPtr leftCorrelation;
  MultiVariateRegressionStatisticsPtr rightCorrelation;

};

} /* namespace lbcpp */

#endif //!ML_DATA_BINARY_SEARCH_TREE_H_
