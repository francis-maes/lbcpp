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
#include <iostream>
#include "precompiled.h"
# include "../../projects/MOO/HoeffdingTreeLearner.h"

namespace lbcpp
{

class HoeffdingTreeNodeStatistics : public Object
{
public:
	std::vector<AttributeObservation> attributeObservations;

	HoeffdingTreeNodeStatistics(ModelType modelType, int nbAttributes) {
		attributeObservations = std::vector<AttributeObservation>(nbAttributes);
		for(unsigned i = 0; i < nbAttributes; i++){
			attributeObservations[i] = AttributeObservation(modelType, i);
		}
	}
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeNodeStatistics> HoeffdingTreeNodeStatisticsPtr;

class HoeffdingTreeIncrementalLearner : public IncrementalLearner
{
public:
	double delta; /* confidence level */
	int chunkSize; /* number of samples before tree is recalculated */
	bool pruneOnly; /* whether to prune only or to generate alternate trees for drift detection */
	double threshold; /* threshold for splitting criterium */
	int verbosity;
	HoeffdingTreeNodePtr root;
	lbcpp::ExecutionContext& context;
	ModelType modelType;
	SplitType splitType;
	IncrementalLearnerPtr perceptronLearner;

	int seenExamples; /* number of unprocessed examples */

  HoeffdingTreeIncrementalLearner(lbcpp::ExecutionContext& context, int seed, ModelType modelType, SplitType splitType, double delta) : context(context), modelType(modelType), splitType(splitType), delta(delta) {
	chunkSize = 50;// 1
	pruneOnly = true;
	threshold = 0.05;
	verbosity = 0; // 3
	seenExamples = 0;

	srand(seed);
	perceptronLearner = perceptronIncrementalLearner(10, 2, 0.05);
}

  HoeffdingTreeIncrementalLearner() : context(defaultExecutionContext()) {}

  ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const {
		const_cast<HoeffdingTreeIncrementalLearner*>(this)->root = new HoeffdingTreeNode(perceptronLearner->createExpression(context, doubleClass), NULL);
		return root;
	}


  void addTrainingSample(ExecutionContext& context, ExpressionPtr expr, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    /*HoeffdingTreeStatisticsPtr stats = root->getLearnerStatistics().staticCast<HoeffdingTreeStatistics>();

    // zoek leaf waar dit voorbeeld behoord
    TreeNodePtr leaf = root->findLeaf(input);
    
    // bereken split
    size_t splitIdx = 0;
    double splitValue = 0.0;

    
    leaf->split(context, splitIdx, splitValue);*/

	  TreeNodePtr root = expr.staticCast<TreeNode>();

	  if(verbosity >= 2) {
		std::cout << "add training sample\n";
	}
	const_cast<HoeffdingTreeIncrementalLearner*>(this)->seenExamples++;
	HoeffdingTreeNodePtr leaf = findLeaf(input);
	updateStatistics(input, output, *leaf);
	/*vector<double> normSample(sample);
	normalizeSample(normSample, *leaf);*/
	updateLinearModel(input, output, *leaf);// <- normalize it
	if(seenExamples % chunkSize == 0){
		// find two best splits Sa and Sb
		if(verbosity >= 3) {
			cout << "try split\n";
		}
		std::vector<Split> bestSplits = *findBestSplitPerAttribute(*leaf, input->getNumElements());
		int indexBestSplit = 0;
		double Sa = 0;
		double Sb = 0;
		for(unsigned i = 0; i < bestSplits.size(); i++){
			if(bestSplits[i].quality > Sa){
				indexBestSplit = i;
				Sb = Sa;
				Sa = bestSplits[i].quality;
			}
			else if(bestSplits[i].quality > Sb){
				Sb = bestSplits[i].quality;
			}
		}
		Sa = Sa==0?1:Sa;
		Sb = Sb==0?1:Sb;

		// splitting?
		double epsilon = MathUtils::hoeffdingBound(1,leaf->examplesSeen, delta);
		bool splitWasMade = false;
		int nbOfLeavesSplit = 0;
		if((Sa != 0 && ((Sb/Sa < 1 - epsilon)|| epsilon < threshold ))){//|| epsilon < threshold
			nbOfLeavesSplit = leaf->examplesSeen;
			HoeffdingTreeNodePtr splittedNode = makeSplit(bestSplits[indexBestSplit], *leaf);
			/*if(verbosity >= 3){
				cout <<"----->split :x" << bestSplits[indexBestSplit].attributeNb << " <= "<< bestSplits[indexBestSplit].value << "\n";
				if(leaf->isRoot())
					cout << "leaf is root";
				else
					cout <<"----->parent :x" << leaf->parent->split->attributeNb << " <= "<< leaf->parent->split->value << "\n";
				cout << "splittednode: " << splittedNode->split->value << "\n";
			}*/
			swap(leaf, splittedNode); // <-----------------------------------------------------------------------------------------------TODO: check if correct swap is called
			//delete leaf;
			splitWasMade = true;
		}
		else{
			splitWasMade = false;
			nbOfLeavesSplit = -1;
		}
		context.resultCallback("bestSplitQuality0", Sa);
		context.resultCallback("bestSplitQuality1", Sb);
		context.resultCallback("splitQualityRatio", Sb/Sa);
		context.resultCallback("hoeffdingBound", epsilon);
		context.resultCallback("Split?", splitWasMade);
		context.resultCallback("SplitNbOfLeaves", (double) nbOfLeavesSplit);
	}
	for(unsigned i = 0; i < input->getNumElements(); i++){
			std::stringstream strs;
			strs << i;
			std::string temp_str = strs.str();
			char* char_type = (char*) temp_str.c_str();
			context.resultCallback(char_type, input->getValue(i));
	}
	context.resultCallback("targetValue", output->getValue(0));

  }

  void swap(HoeffdingTreeNodePtr originalNode, HoeffdingTreeNodePtr newNode) const {
	if(originalNode->isRoot()) 
		*(const_cast<HoeffdingTreeIncrementalLearner*>(this)->root) = *newNode;
	else if(originalNode->parent->getLeft() == originalNode)
		originalNode->parent->setLeft(newNode);
	else
		originalNode->parent->setRight(newNode);
}

  double predict(const DenseDoubleVectorPtr& input) const {
	HoeffdingTreeNodePtr leaf = findLeaf(input);
	//vector<double> normSample(sample);
	//normalizeSample(normSample, *leaf);
	//return leaf->linearModel->compute
	//DenseDoubleVectorPtr sample = new DenseDoubleVector(*input);
	//sample->appendValue(output->getValue(0));
	//return perceptronIncrementalLearner->
	return leaf->linearModel->compute(input);
}

void pprint() const {
	root->pprint();
}

  HoeffdingTreeNode* makeSplit(const Split& split, const HoeffdingTreeNode& leaf) const{
	if(verbosity >= 1) {
		cout << "split was made at leaf:\n";
		leaf.pprint();
	}
	HoeffdingTreeNode* leftChild = new HoeffdingTreeNode(leaf.linearModel, NULL);
	HoeffdingTreeNode* rightChild = new HoeffdingTreeNode(leaf.linearModel, NULL);
	HoeffdingTreeNode* parent = new HoeffdingTreeNode(split.attributeNb, split.value, leftChild, rightChild, leaf.parent);
	leftChild->parent = parent;
	rightChild->parent = parent;
	//delete leaf;
	//leaf = parent;
	//cout << "splitting proc \n";
	/*if(leaf->parent != NULL){
	cout << (leaf->parent->left != NULL) << "\n";
	cout << (leaf->parent->right != NULL) << "\n";
	if(leaf->parent->left == leaf)
		leaf->parent->left = parent;
	else
		leaf->parent->right = parent;
	}
	else
	{
		leaf = static_cast<InternalNode*>(parent);
	}*/
	return parent;
}

  HoeffdingTreeNodePtr findLeaf(const DenseDoubleVectorPtr& input) const
  {
	// TODO: drift detection code
	return root->findLeaf(input);
}

  std::vector<Split>* findBestSplitPerAttribute(HoeffdingTreeNode& leaf, int nbAttributes) const{
	std::vector<Split>* bestSplits = new std::vector<Split>();
	for(unsigned i = 0; i < nbAttributes; i++){
		HoeffdingTreeNodeStatisticsPtr stats = leaf.getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
		AttributeObservation observation = stats->attributeObservations[i];
		bestSplits->push_back(*observation.findBestSplitPoint());
	}
	return bestSplits;
}


  void updateLinearModel(const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output, HoeffdingTreeNode& leaf) const {
	if(verbosity >= 2) {
		cout << "update linear model\n";
	}
	/*DenseDoubleVector test = DenseDoubleVector(input);
	test.appendValue(output->getValue(0));
	test.getValues();*/
	perceptronLearner->addTrainingSample(context, leaf.linearModel, input, output); //->addTrainingSample(context, leaf.linearModel, input, output);
	//leaf.linearModel->train(sample);
}

  void updateStatistics(const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output, HoeffdingTreeNode& leaf) const {
	if(verbosity >= 2) {
		std::cout << "update statistics \n";
	}
	leaf.examplesSeen++;
	// update models of attributes
	if(leaf.getLearnerStatistics() == NULL){
		leaf.setLearnerStatistics(new HoeffdingTreeNodeStatistics(modelType, input->getNumElements()));
	}
	for(unsigned i = 0; i < input->getNumElements(); i++){
		HoeffdingTreeNodeStatisticsPtr stats = leaf.getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
		stats->attributeObservations[i].model->add(input->getValue(i), output->getValue(0));
	}
}

  	  // debug
	DenseDoubleVectorPtr getSplits() {
		return (this->root)->getSplits();
	}

protected:
  friend class HoeffdingTreeIncrementalLearnerClass;
};

typedef ReferenceCountedObjectPtr<HoeffdingTreeIncrementalLearner> HoeffdingTreeIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_HOEFFDING_TREE_LEARNER_H_
