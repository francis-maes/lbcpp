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
  HoeffdingTreeIncrementalLearnerStatistics(size_t numAttributes = 0) : ebsts(std::vector<ExtendedBinarySearchTreePtr>(numAttributes))
  {
    for (size_t i = 0; i < numAttributes; ++i)
      ebsts[i] = new ExtendedBinarySearchTree();
  }

  // copies one side of the EBST for the splitAttribute and copies the complete EBST for all other attribute
  HoeffdingTreeIncrementalLearnerStatistics(ExecutionContext& context, IncrementalLearnerStatisticsPtr parentStats, size_t attribute, double splitValue, bool leftSide)
  {
    HoeffdingTreeIncrementalLearnerStatisticsPtr stats = parentStats.staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    size_t numAttributes = parentStats != NULL ? numAttributes = stats->ebsts.size() : 0;
    ebsts = std::vector<ExtendedBinarySearchTreePtr>(numAttributes);
    for (size_t i = 0; i < numAttributes; ++i)
    {
      if(i != attribute)
        ebsts[i] = stats->ebsts[i]->clone(context);
      else
      {
        ExtendedBinarySearchTreePtr node = stats->ebsts[i]->getNode(splitValue).staticCast<ExtendedBinarySearchTree>();
        if(leftSide)
        {
          if(node->isLeftChild())
          {
            if(node->getLeft().exists())
              ebsts[i] = node->getLeft()->clone(context);
            else
              ebsts[i] = new ExtendedBinarySearchTree();
          }
          else if(node->isRightChild())
          {
            ebsts[i] = node->clone(context);
            if(node->getLeft().exists())
              ebsts[i]->getRight() = node->getLeft()->clone(context);
            else
              ebsts[i]->getRight() = new ExtendedBinarySearchTree();
          }
          else
          {
            if(stats->ebsts[i]->getLeft().exists())
              ebsts[i] = stats->ebsts[i]->getLeft()->clone(context);
            else
              ebsts[i] = new ExtendedBinarySearchTree();
          }
        }
        else
        {
          if(node->isLeftChild())
          {
            ebsts[i] = node->clone(context);
            if(stats->ebsts[i]->getRight().exists())
              ebsts[i]->getLeft() = stats->ebsts[i]->getRight()->clone(context);
            else
              ebsts[i]->getLeft() = new ExtendedBinarySearchTree();
          }
          else if(node->isRightChild())
          {
            if(node->getRight().exists())
              ebsts[i] = node->getRight()->clone(context);
            else
              ebsts[i] = new ExtendedBinarySearchTree();
          }
          else
          {
            if(stats->ebsts[i]->getRight().exists())
              ebsts[i] = stats->ebsts[i]->getRight()->clone(context);
            else
              ebsts[i] = new ExtendedBinarySearchTree();
          }
        }
      }
    }
  }

  void addObservation(const DenseDoubleVectorPtr& attributes, double target)
  {
    incrementExamplesSeen();
    if (ebsts.size() == 0)
      for (size_t i = 0; i < attributes->getNumValues(); ++i)
        ebsts.push_back(new ExtendedBinarySearchTree());
    jassert(attributes->getNumValues() == ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      ebsts[i]->insertValue(attributes->getValue(i), target);
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    HoeffdingTreeIncrementalLearnerStatisticsPtr result = new HoeffdingTreeIncrementalLearnerStatistics();
    result->ebsts = std::vector<ExtendedBinarySearchTreePtr>(ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      result->ebsts[i] = ebsts[i]->clone(context);
    return result;
  }

  std::vector<ObjectPtr> getRegressionStatsForSplit(size_t attribute, double splitValue, bool leftSide)
  {
    std::vector<ObjectPtr> result;
    for (size_t i = 0; i < ebsts.size(); ++i)
    {
      PearsonCorrelationCoefficientPtr stats = new PearsonCorrelationCoefficient();
      if (i != attribute)
      {
        stats->update(*(ebsts[i]->leftCorrelation));
        stats->update(*(ebsts[i]->rightCorrelation));
      }
      else
      {
        if (leftSide)
          stats->update(*(ebsts[i]->getStatsForSplit(splitValue).first));
        else
          stats->update(*(ebsts[i]->getStatsForSplit(splitValue).second));
      }
      result.push_back(stats);
    }
    return result;
  }

  const std::vector<ExtendedBinarySearchTreePtr>& getEBSTs() const
    {return ebsts;}

protected:
  friend class HoeffdingTreeIncrementalLearnerStatisticsClass;

  std::vector<ExtendedBinarySearchTreePtr> ebsts;
};

class HoeffdingTreeIncrementalLearner : public IncrementalLearner
{
public:
  HoeffdingTreeIncrementalLearner() {}

	HoeffdingTreeIncrementalLearner(IncrementalSplittingCriterionPtr splittingCriterion, IncrementalLearnerPtr modelLearner, size_t chunkSize) : 
    splittingCriterion(splittingCriterion), modelLearner(modelLearner), chunkSize(chunkSize), pruneOnly(true) {}

  ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const 
  {
    HoeffdingTreeNodePtr result = new HoeffdingTreeNode(modelLearner->createExpression(context, doubleClass), HoeffdingTreeNodePtr());
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
    if (verbosity >= verbosityDetailed)
      context.resultCallback("model", leaf->getModel()->clone(context));
	  
    if (leafStats->getExamplesSeen() % chunkSize == 0) // >= ipv %
    {
      IncrementalSplittingCriterion::Split split = splittingCriterion->findBestSplit(leaf);
      bool splitWasMade = false;

      if (split.value != DVector::missingValue && split.quality != DVector::missingValue)
      {
        OVectorPtr leftRegressionStats = new OVector();
        OVectorPtr rightRegressionStats = new OVector();
        leftRegressionStats->getNativeVector() = leafStats->getRegressionStatsForSplit(split.attribute, split.value, true);
        rightRegressionStats->getNativeVector() = leafStats->getRegressionStatsForSplit(split.attribute, split.value, false);
        leaf->split(context, split.attribute, split.value);
        modelLearner->initialiseLearnerStatistics(context, leaf->getLeft().staticCast<HoeffdingTreeNode>()->getModel(), leftRegressionStats);
        modelLearner->initialiseLearnerStatistics(context, leaf->getRight().staticCast<HoeffdingTreeNode>()->getModel(), rightRegressionStats);


        /*(leaf->getLeft()).staticCast<HoeffdingTreeNode>()->getPerceptron()->getModel()->getWeights()->setValue(0, split.leftThresholdWeight);
        (leaf->getRight()).staticCast<HoeffdingTreeNode>()->getPerceptron()->getModel()->getWeights()->setValue(0, split.rightThresholdWeight);
        (leaf->getLeft()).staticCast<HoeffdingTreeNode>()->getPerceptron()->getModel()->getWeights()->setValue(split.attribute, split.leftAttributeWeight);
        (leaf->getRight()).staticCast<HoeffdingTreeNode>()->getPerceptron()->getModel()->getWeights()->setValue(split.attribute, split.rightAttributeWeight);*/
			  splitWasMade = true;
		  }
      
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("bestSplitQuality", split.quality);
		    context.resultCallback("Split?", splitWasMade);
        context.resultCallback("Splitvalue", split.value);
      }
	  }
  
    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("observation", input);
	    context.resultCallback("targetValue", output->getValue(0));
    }
  }

protected:
  friend class HoeffdingTreeIncrementalLearnerClass;

  IncrementalLearnerPtr modelLearner;
  IncrementalSplittingCriterionPtr splittingCriterion;
  size_t chunkSize; /* number of samples before tree is recalculated */
	bool pruneOnly; /* whether to prune only or to generate alternate trees for drift detection */
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeIncrementalLearner> HoeffdingTreeIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_HOEFFDING_TREE_LEARNER_H_
