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

namespace lbcpp
{

enum ModelType {NY, NXY};
enum SplitType {Hoeffding, FStatistic};

class HoeffdingTreeNodeStatistics;
typedef ReferenceCountedObjectPtr<HoeffdingTreeNodeStatistics> HoeffdingTreeNodeStatisticsPtr;

class HoeffdingTreeNodeStatistics : public Object
{
public:
	/*std::vector<AttributeObservation> attributeObservations;

	HoeffdingTreeNodeStatistics(ModelType modelType, int nbAttributes) {
		attributeObservations = std::vector<AttributeObservation>(nbAttributes);
		for(int i = 0; i < nbAttributes; i++){
			attributeObservations[i] = AttributeObservation(modelType, i);
		}
	}*/

  HoeffdingTreeNodeStatistics(size_t numAttributes = 0) : ebsts(std::vector<ExtendedBinarySearchTreePtr>(numAttributes))
  {
    for (size_t i = 0; i < numAttributes; ++i)
      ebsts[i] = new ExtendedBinarySearchTree();
  }

  void addObservation(const DenseDoubleVectorPtr& attributes, double target)
  {
    if (ebsts.size() == 0)
      for (size_t i = 0; i < attributes->getNumValues(); ++i)
        ebsts.push_back(new ExtendedBinarySearchTree());
    jassert(attributes->getNumValues() == ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      ebsts[i]->insertValue(attributes->getValue(i), target);
  }

  size_t getNumExamplesSeen()
    {return (size_t) ebsts[0]->getLeftStats()->getCount() + (size_t) ebsts[0]->getRightStats()->getCount();}

  /** Find the best split point for each attribute.
   *  \return A vector where each element in the vector is a pair, where the first element is the split point, and
   *  the second element is the split quality. The index of the pair in the vector corresponds
   *  to the index of the attribute.
   */
  std::vector< std::pair<double,double> > findBestSplitPoints()
  {
    std::vector< std::pair<double,double> > result(ebsts.size());
    for (size_t i = 0; i < result.size(); ++i)
      result[i] = ebsts[i]->findBestSplitPoint();
    return result;
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    HoeffdingTreeNodeStatisticsPtr result = new HoeffdingTreeNodeStatistics();
    result->ebsts = std::vector<ExtendedBinarySearchTreePtr>(ebsts.size());
    for (size_t i = 0; i < ebsts.size(); ++i)
      result->ebsts[i] = ebsts[i]->clone(context);
    return result;
  }

protected:
  friend class HoeffdingTreeNodeStatisticsClass;

  std::vector<ExtendedBinarySearchTreePtr> ebsts;
};

class HoeffdingTreeIncrementalLearner : public IncrementalLearner
{
public:
	double delta; /* confidence level */
	size_t chunkSize; /* number of samples before tree is recalculated */
	bool pruneOnly; /* whether to prune only or to generate alternate trees for drift detection */
	double threshold; /* threshold for splitting criterium */
	ModelType modelType;
	SplitType splitType;
	IncrementalLearnerPtr perceptronLearner;

  HoeffdingTreeIncrementalLearner(double delta) : delta(delta)
  {
	  chunkSize = 10;// 1
	  pruneOnly = true;
	  threshold = 0.05;
	  perceptronLearner = perceptronIncrementalLearner(10, 2, 0.05);
  }

  HoeffdingTreeIncrementalLearner() 
  {
    chunkSize = 10;// 1
	  pruneOnly = true;
	  threshold = 0.05;
	  perceptronLearner = perceptronIncrementalLearner(10, 2, 0.05);
  }

  ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const 
  {
    HoeffdingTreeNodePtr result = new HoeffdingTreeNode(perceptronLearner->createExpression(context, doubleClass), HoeffdingTreeNodePtr());
    result->setLearnerStatistics(new HoeffdingTreeNodeStatistics());
    return result;
  }


  void addTrainingSample(ExecutionContext& context, ExpressionPtr expr, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    jassert(output->getNumValues() == 1);
    HoeffdingTreeNodePtr root = expr.staticCast<HoeffdingTreeNode>();
    HoeffdingTreeNodePtr leaf = root->findLeaf(input);
    HoeffdingTreeNodeStatisticsPtr leafStats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
    leafStats->addObservation(input, output->getValue(0));
    perceptronLearner->addTrainingSample(context, leaf->getPerceptron(), input, output);
	  
    if (leafStats->getNumExamplesSeen() >= chunkSize) // >= ipv %
    {
      std::vector< std::pair<double,double> > bestSplits = leafStats->findBestSplitPoints();
		  int indexBestSplit = 0;
		  double Sa = 0;
		  double Sb = 0;
		  for (size_t i = 0; i < bestSplits.size(); ++i)
      {
			  if (bestSplits[i].second > Sa)
        {
				  indexBestSplit = i;
				  Sb = Sa;
				  Sa = bestSplits[i].second;
			  }
			  else if (bestSplits[i].second > Sb)
				  Sb = bestSplits[i].second;
		  }
		  Sa = (Sa == 0 ? 1 : Sa);
		  Sb = (Sb == 0 ? 1 : Sb);

		  // splitting?
      double epsilon = hoeffdingBound(1, leafStats->getNumExamplesSeen(), delta);
		  bool splitWasMade = false;
		  if ( Sa != 0 && ( Sb/Sa < (1 - epsilon) || epsilon < threshold))
      {
        leaf->split(context, indexBestSplit, bestSplits[indexBestSplit].first);
			  splitWasMade = true;
		  }
      
      if (verbosity >= verbosityDetailed)
      {
		    context.resultCallback("bestSplitQuality0", Sa);
		    context.resultCallback("bestSplitQuality1", Sb);
		    context.resultCallback("splitQualityRatio", Sb/Sa);
		    context.resultCallback("hoeffdingBound", epsilon);
		    context.resultCallback("Split?", splitWasMade);
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

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}

  inline double log2(double n) const
    {return log(n) / log(2.0);}

protected:
  friend class HoeffdingTreeIncrementalLearnerClass;
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeIncrementalLearner> HoeffdingTreeIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_HOEFFDING_TREE_LEARNER_H_
