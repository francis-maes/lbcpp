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
# include <iostream>
# include "precompiled.h"
# include <ml/BinarySearchTree.h>
# include <map>

namespace lbcpp
{

class HoeffdingTreeIncrementalLearnerStatistics;
typedef ReferenceCountedObjectPtr<HoeffdingTreeIncrementalLearnerStatistics> HoeffdingTreeIncrementalLearnerStatisticsPtr;

class HoeffdingTreeIncrementalLearnerStatistics : public IncrementalLearnerStatistics
{
public:
  HoeffdingTreeIncrementalLearnerStatistics(size_t numAttributes = 0) : ebsts(std::vector<ExtendedBinarySearchTreePtr>(numAttributes)), splitRatios(new ScalarVariableMean())
  {
    for (size_t i = 0; i < numAttributes; ++i)
      ebsts[i] = new ExtendedBinarySearchTree();
  }

  void addObservation(const DenseDoubleVectorPtr& attributes, double target)
  {
    incrementExamplesSeen();
    if (ebsts.size() == 0)
      for (size_t i = 0; i < attributes->getNumValues(); ++i)
        ebsts.push_back(new ExtendedBinarySearchTree());
    jassert(attributes->getNumValues() == ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      ebsts[i]->insertValue(attributes->getValue(i), attributes, target);
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    HoeffdingTreeIncrementalLearnerStatisticsPtr result = new HoeffdingTreeIncrementalLearnerStatistics();
    result->ebsts = std::vector<ExtendedBinarySearchTreePtr>(ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      result->ebsts[i] = ebsts[i]->clone(context);
    return result;
  }

  const std::vector<ExtendedBinarySearchTreePtr>& getEBSTs() const
    {return ebsts;}

  ScalarVariableMeanPtr getSplitRatios() const
    {return splitRatios;}

  /* Reset the split ratio statistic */
  void resetSplitRatios(IncrementalSplittingCriterion::Split newBestSplit, IncrementalSplittingCriterion::Split newSecondBestSplit)
  {
    splitRatios = new ScalarVariableMean();
    bestSplit = newBestSplit;
    secondBestSplit = newSecondBestSplit;
  }

  size_t getTotalEBSTNodes() const
  {
    size_t result = 0;
    for (size_t i = 0; i < ebsts.size(); ++i)
      result += ebsts[i]->getSize();
    return result;
  }

  IncrementalSplittingCriterion::Split getBestSplit() const
    {return bestSplit;}

  IncrementalSplittingCriterion::Split getSecondBestSplit() const
    {return secondBestSplit;}

protected:
  friend class HoeffdingTreeIncrementalLearnerStatisticsClass;

  std::vector<ExtendedBinarySearchTreePtr> ebsts;
  ScalarVariableMeanPtr splitRatios;
  IncrementalSplittingCriterion::Split bestSplit, secondBestSplit;
};

class HoeffdingTreeIncrementalLearner : public IncrementalLearner
{
public:
  HoeffdingTreeIncrementalLearner() {}

	HoeffdingTreeIncrementalLearner(IncrementalSplittingCriterionPtr splittingCriterion, IncrementalLearnerPtr modelLearner) : 
    splittingCriterion(splittingCriterion), modelLearner(modelLearner), pruneOnly(true) {}

  ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const 
  {
    HoeffdingTreeNodePtr result = new HoeffdingTreeNode(modelLearner->createExpression(context, doubleClass));
    result->setLearnerStatistics(new HoeffdingTreeIncrementalLearnerStatistics());
    return result;
  }


  void addTrainingSample(ExecutionContext& context, ExpressionPtr expr, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    jassert(output->getNumValues() == 1);
    HoeffdingTreeNodePtr root = expr.staticCast<HoeffdingTreeNode>();

    HoeffdingTreeNodePtr leaf = root->findLeaf(input);
    HoeffdingTreeIncrementalLearnerStatisticsPtr leafStats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    
    modelLearner->addTrainingSample(context, leaf->getModel(), input, output);
    
    leafStats->addObservation(input, output->getValue(0));
    if (verbosity >= verbosityAll)
      context.resultCallback("model", root);

    IncrementalSplittingCriterion::Split split = splittingCriterion->findBestSplit(context, leaf);
    bool splitWasMade = false;

    if (split.value != DVector::missingValue && split.attribute != DVector::missingValue)
    {
      std::pair<MultiVariateRegressionStatisticsPtr, MultiVariateRegressionStatisticsPtr> regressionStats = leafStats->getEBSTs()[split.attribute]->getStatsForSplit(split.value);
      leaf->split(context, split.attribute, split.value);
      modelLearner->initialiseLearnerStatistics(context, leaf->getLeft().staticCast<HoeffdingTreeNode>()->getModel(), regressionStats.first);
      modelLearner->initialiseLearnerStatistics(context, leaf->getRight().staticCast<HoeffdingTreeNode>()->getModel(), regressionStats.second);
      leaf->setLearnerStatistics(HoeffdingTreeIncrementalLearnerStatisticsPtr());
			splitWasMade = true;
		}
    
    if (callback.exists())
     callback->exampleAdded(context, root);

    /*if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("bestSplitQuality", split.quality);
		  context.resultCallback("Split?", splitWasMade);
      context.resultCallback("Splitvalue", split.value);
      context.resultCallback("SplitIdx", split.attribute);
      context.resultCallback("nbOfModels", root->getNbOfLeaves());
      std::vector<TreeNodePtr> leaves = root->getAllLeafs();
      size_t totalEBSTNodes = 0;
      for (size_t i = 0; i < leaves.size(); ++i)
        totalEBSTNodes += leaves[i]->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>()->getTotalEBSTNodes();
      context.resultCallback("nbEBSTNodes", totalEBSTNodes);
    }*/
  }

protected:
  friend class HoeffdingTreeIncrementalLearnerClass;

  IncrementalLearnerPtr modelLearner;
  IncrementalSplittingCriterionPtr splittingCriterion;
	bool pruneOnly; /* whether to prune only or to generate alternate trees for drift detection */
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeIncrementalLearner> HoeffdingTreeIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_HOEFFDING_TREE_LEARNER_H_
