/*-----------------------------------------.---------------------------------.
| Filename: HoeffdingTreeLearner.h         | Hoeffding Tree Learner          |
| Author  : Denny Verbeeck                 | Incremental Model Tree Learner  |
| Started : 03/12/2013 13:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_HOEFFDING_TREE_LEARNER_H_
# define ML_HOEFFDING_TREE_LEARNER_H_

# include <ml/IncrementalLearner.h>
# include <ml/Expression.h>

namespace lbcpp
{

class HoeffdingTreeStatistics : public Object
{
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeStatistics> HoeffdingTreeStatisticsPtr;

class HoeffdingTreeIncrementalLearner : public IncrementalLearner
{
public:
  HoeffdingTreeIncrementalLearner() {}
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expr, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    TreeNodePtr root = expr.staticCast<TreeNode>();
    HoeffdingTreeStatisticsPtr stats = root->getLearnerStatistics().staticCast<HoeffdingTreeStatistics>();

    // zoek leaf waar dit voorbeeld behoord
    TreeNodePtr leaf = root->findLeaf(input);
    
    // bereken split
    size_t splitIdx = 0;
    double splitValue = 0.0;

    
    leaf->split(context, splitIdx, splitValue);
  }


  // een lege root maken
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return hoeffdingTreeNode();}

protected:
  friend class HoeffdingTreeIncrementalLearnerClass;
};

} /* namespace lbcpp */

#endif //!ML_HOEFFDING_TREE_LEARNER_H_
