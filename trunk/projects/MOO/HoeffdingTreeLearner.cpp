//============================================================================
// Name        : HoeffdingTreeLearner.cpp
// Author      : Xavier Rasschaert
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "HoeffdingTreeLearner.h"
using namespace std;

HoeffdingTreeLearner::HoeffdingTreeLearner(lbcpp::ExecutionContext& context, double delta, const DataDefinition& dataDefinition) : context(context) {
	this->delta = delta;
	this->dataDefinition = &dataDefinition;
	root = new LeafNode(dataDefinition, initialLearningRate, learningRateDecay, NULL);
};

HoeffdingTreeLearner::~HoeffdingTreeLearner(){};

void HoeffdingTreeLearner::addTrainingSample(const vector<float>& sample) {
	if(verbosity >= 2) {
		cout << "add training sample\n";
	}
	seenExamples++;
	LeafNode* leaf = traverseSample(sample);
	updateStatistics(sample, *leaf);
	updateLinearModel(sample, *leaf);
	if(seenExamples % chunkSize == 0){
		// find two best splits Sa and Sb
		cout << "try split\n";
		vector<Split> bestSplits = *findBestSplitPerAttribute(*leaf);
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
		double epsilon = MathUtils::hoeffdingBound(1,leaf->n, delta);
		cout << "epsilon: " << epsilon << "n: " << leaf->n << "\n";
		cout << "splitquality: " << Sb/Sa << "\n";
		if(Sa != 0 && ((Sb/Sa < 1 - epsilon) )){//|| epsilon < threshold
			nbOfLeavesSplit = leaf->n;
			cout <<"----->split :x" << bestSplits[indexBestSplit].attributeNb << " <= "<< bestSplits[indexBestSplit].value << "\n";
			if(leaf->isRoot())
				cout << "leaf is root";
			else
				cout <<"----->parent :x" << leaf->parent->split->attributeNb << " <= "<< leaf->parent->split->value << "\n";
			InternalNode* splittedNode = makeSplit(bestSplits[indexBestSplit], *leaf);
			cout << "splittednode: " << splittedNode->split->value << "\n";
			swap(*leaf, *splittedNode);
			//delete leaf;
			cout << "splittednode: " << static_cast<InternalNode*>(root)->split->value << "\n";
			splitWasMade = true;
		}
		else{
			splitWasMade = false;
			nbOfLeavesSplit = -1;
		}
	}

}

LeafNode* HoeffdingTreeLearner::traverseSample(const vector<float>& sample) const {
	// TODO: drift detection code

	return root->traverseSample(sample);
}

void HoeffdingTreeLearner::updateStatistics(const vector<float>& sample, LeafNode& leaf) {
	if(verbosity >= 2) {
		cout << "update statistics \n";
	}
	leaf.n++;
	double y = sample[sample.size() - 1];
	// update models of nominal attributes
	for(unsigned i = 0; i < leaf.nominalAttributeObservations.size(); i++){
		DerivedModel m = leaf.nominalAttributeObservations[i].models[sample[dataDefinition->nominalAttributeDefinitions[i].index]];
		m.update(1,y,y*y);
	}
	// update models of numerical attributes
	for(unsigned i = 0; i < leaf.numericalAttributeObservations.size(); i++){
		leaf.numericalAttributeObservations[i].model->add(sample[dataDefinition->numericalAttributeDefinitions[i].index], y);
	}
}

void HoeffdingTreeLearner::updateLinearModel(const vector<float>& sample, LeafNode& leaf) {
	if(verbosity >= 2) {
		cout << "update linear model\n";
	}
	leaf.linearModel->train(sample);
}

vector<Split>* HoeffdingTreeLearner::findBestSplitPerAttribute(const LeafNode& leaf) const{
	vector<Split>* bestSplits = new vector<Split>();
	for(unsigned i = 0; i < dataDefinition->nominalAttributeDefinitions.size(); i++){
		NominalAttributeObservation observation = leaf.nominalAttributeObservations[i];
		bestSplits->push_back(*observation.findBestSplitPoint());
	}
	for(unsigned i = 0; i < dataDefinition->numericalAttributeDefinitions.size(); i++){
		NumericalAttributeObservation observation = leaf.numericalAttributeObservations[i];
		bestSplits->push_back(*observation.findBestSplitPoint());
	}
	return bestSplits;
}

InternalNode* HoeffdingTreeLearner::makeSplit(const Split& split, const LeafNode& leaf) {
	if(verbosity >= 1) {
		cout << "split was made at leaf:\n";
		leaf.pprint();
	}
	LeafNode* leftChild = new LeafNode(*dataDefinition, *(leaf.linearModel), NULL);
	LeafNode* rightChild = new LeafNode(*dataDefinition, *(leaf.linearModel), NULL);
	InternalNode* parent = new InternalNode(new Split(split), leftChild, rightChild, leaf.parent);
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
	cout << "reached the end of splitting proc \n";
	return parent;
}

void HoeffdingTreeLearner::swap(Node& originalNode, Node& newNode) {
	cout << "begin swap \n";
	if(originalNode.isRoot())
		root = &newNode;
	else if(originalNode.parent->left == &originalNode)
		originalNode.parent->left = &newNode;
	else
		originalNode.parent->right = &newNode;
	cout << "end swap \n";
}

double HoeffdingTreeLearner::predict(const vector<float>& sample) const {
	LeafNode* leaf = traverseSample(sample);
	return leaf->linearModel->predict(sample);
}

void HoeffdingTreeLearner::pprint() const {
	(this->root)->pprint();
}

// example thesis Elena Ivonomovska
/*void testEBSTAdd(){
	EBST* bst = new EBST();
	bst->add(0.95, 0.71);
	bst->add(0.43, 0.56);
	bst->add(7.44, 2.3);
	bst->add(1.03, 0.18);
	bst->add(8.25, 3.45);
	cout << bst->leftModel->Sy << " " << bst->rightModel->Sy<< "\n";
	cout << bst->leftModel->Syy << " " << bst->rightModel->Syy << "\n";
	cout << bst->leftModel->n << " " << bst->rightModel->n << "\n";
	cout << bst->key << "\n";
	cout << "*********************" << "\n";
	cout << bst->left->leftModel->Sy << " " << bst->left->rightModel->Sy<< "\n";
	cout << bst->left->leftModel->Syy << " " << bst->left->rightModel->Syy << "\n";
	cout << bst->left->leftModel->n << " " << bst->left->rightModel->n << "\n";
	cout << bst->left->key << "\n";
	cout << "*********************" << "\n";
	cout << bst->right->leftModel->Sy << " " << bst->right->rightModel->Sy<< "\n";
	cout << bst->right->leftModel->Syy << " " << bst->right->rightModel->Syy << "\n";
	cout << bst->right->leftModel->n << " " << bst->right->rightModel->n << "\n";
	cout << bst->right->key << "\n";
	cout << "*********************" << "\n";
	cout << bst->right->left->leftModel->Sy << " " << bst->right->left->rightModel->Sy<< "\n";
	cout << bst->right->left->leftModel->Syy << " " << bst->right->left->rightModel->Syy << "\n";
	cout << bst->right->left->leftModel->n << " " << bst->right->left->rightModel->n << "\n";
	cout << bst->right->left->key << "\n";
	cout << "*********************" << "\n";
	cout << bst->right->right->leftModel->Sy << " " << bst->right->right->rightModel->Sy<< "\n";
	cout << bst->right->right->leftModel->Syy << " " << bst->right->right->rightModel->Syy << "\n";
	cout << bst->right->right->leftModel->n << " " << bst->right->right->rightModel->n << "\n";
	cout << bst->right->right->key << "\n";
}*/

// expected splitting point is in the middle : 3
/*void testEBSTFind(){
	EBST* bst = new EBST();
	bst->add(1, 1);
	bst->add(2, 1.25);
	bst->add(3, 1.1);
	bst->add(5, 3.2);
	bst->add(6, 3.25);
	bst->add(7, 3.1);
	double maxSDR;
	double splitPoint;
	bst->findBestSplit(maxSDR, splitPoint);
	cout << maxSDR << "\n";
	cout << splitPoint << "\n";
}*/

// function: f(x1,x2) = 3*x1 + 7*x2 + 2
/*void testPerceptronNumerical(){
	DataDefinition* dataDef = new DataDefinition();
	dataDef->addNumericalAttribute("numAtt1");
	dataDef->addNumericalAttribute("numAtt2");
	dataDef->addTargetAttribute("targetValue");
	HoeffdingTreeLearner* htl = new HoeffdingTreeLearner(NULL, 0.95, *dataDef);
	vector<float>* sample1 = new vector<float>(0);
	sample1->push_back(1.0/5);
	sample1->push_back(2.0/5);
	sample1->push_back(5.4);
	vector<float>* sample2 = new vector<float>(0);
	sample2->push_back(3.0/5);
	sample2->push_back(4.0/5);
	sample2->push_back(9.4);
	vector<float>* sample3 = new vector<float>(0);
	sample3->push_back(2.0/5);
	sample3->push_back(2.0/5);
	sample3->push_back(6);
	vector<float>* sample4 = new vector<float>(0);
	sample4->push_back(2.0/5);
	sample4->push_back(1.0/5);
	sample4->push_back(4.6);
	vector<float>* sample5 = new vector<float>(0);
	sample5->push_back(5.0/5);
	sample5->push_back(3.0/5);
	sample5->push_back(9.2);
	vector<float>* sample6 = new vector<float>(0);
	sample6->push_back(2.0/5);
	sample6->push_back(3.0/5);
	sample6->push_back(7.4);
	for(int i = 0; i < 10; i++)
	{
		htl->addTrainingSample(*sample1);
		htl->addTrainingSample(*sample2);
		htl->addTrainingSample(*sample3);
		htl->addTrainingSample(*sample4);
		htl->addTrainingSample(*sample5);
		htl->addTrainingSample(*sample6);
	}
	cout << "Result:\n";
	cout << (*sample1)[2] << "<->" << htl->predict(*sample1) << "\n";
	cout << (*sample2)[2] << "<->" << htl->predict(*sample2) << "\n";
	cout << (*sample3)[2] << "<->" << htl->predict(*sample3) << "\n";
	cout << (*sample4)[2] << "<->" << htl->predict(*sample4) << "\n";
	cout << (*sample5)[2] << "<->" << htl->predict(*sample5) << "\n";
	cout << (*sample6)[2] << "<->" << htl->predict(*sample6) << "\n";
	LeafNode* leaf = htl->traverseSample(*sample1);
	cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
	cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
	cout << "t: " << leaf->linearModel->threshold << "\n";
	htl->pprint();
}*/

// function: f(x1,x2) = 3*x1 + 7*x2 + 2
/*void testPerceptronNumerical2(){
	DataDefinition* dataDef = new DataDefinition();
	dataDef->addNumericalAttribute("numAtt1");
	dataDef->addNumericalAttribute("numAtt2");
	dataDef->addTargetAttribute("targetValue");
	HoeffdingTreeLearner* htl = new HoeffdingTreeLearner(NULL, 0.95, *dataDef);
	for(int i = 0; i < 20; i++)
	{
		vector<float>* sample = new vector<float>(0);
		sample->push_back(MathUtils::randDouble());
		sample->push_back(MathUtils::randDouble());
		sample->push_back(3*(*sample)[0]+7*(*sample)[1]+2);
		htl->addTrainingSample(*sample);
	}
	cout << "Result:\n";
	vector<float>* sample = new vector<float>(0);
	sample->push_back(MathUtils::randDouble());
	sample->push_back(MathUtils::randDouble());
	LeafNode* leaf = htl->traverseSample(*sample);
	cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
	cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
	cout << "t: " << leaf->linearModel->threshold << "\n";
	htl->pprint();
}*/

// function 1: f1(x1,x2) = 3*x1 + 7*x2 + 2
// function 2: f2(x1,x2) = -5*x1 - 2*x2 + 3
/*void testPerceptronNumericalSplit(){
	DataDefinition* dataDef = new DataDefinition();
	dataDef->addNumericalAttribute("numAtt1");
	dataDef->addNumericalAttribute("numAtt2");
	dataDef->addTargetAttribute("targetValue");
	HoeffdingTreeLearner* htl = new HoeffdingTreeLearner(NULL, 0.95, *dataDef);
	for(int i = 0; i < 80; i++)
	{
		vector<float>* sample1 = new vector<float>(0);
		sample1->push_back(MathUtils::randDouble() /2);
		sample1->push_back(MathUtils::randDouble()/2);
		sample1->push_back(3*(*sample1)[0]+7*(*sample1)[1]+2);
		htl->addTrainingSample(*sample1);
		vector<float>* sample2 = new vector<float>(0);
		sample2->push_back(MathUtils::randDouble()/2+0.5);
		sample2->push_back(MathUtils::randDouble()/2+0.5);
		sample2->push_back(-5*(*sample2)[0]-2*(*sample2)[1]+3);
		htl->addTrainingSample(*sample2);
	}
	cout << "Result:\n";
	vector<float>* sample = new vector<float>(0);
	sample->push_back(MathUtils::randDouble()/2);
	sample->push_back(MathUtils::randDouble()/2);
	LeafNode* leaf = htl->traverseSample(*sample);
	cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
	cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
	cout << "t: " << leaf->linearModel->threshold << "\n";
	htl->pprint();
}*/

/*void testPerceptronNominal(){
	DataDefinition* dataDef = new DataDefinition();
	dataDef->addNominalAttribute("numAtt1", 2);
	dataDef->addNominalAttribute("numAtt2", 2);
	dataDef->addTargetAttribute("targetValue");
	HoeffdingTreeLearner* htl = new HoeffdingTreeLearner(NULL, 0.95, *dataDef);
	vector<float>* sample1 = new vector<float>(0);
	sample1->push_back(0);
	sample1->push_back(0);
	sample1->push_back(1.5);
	vector<float>* sample2 = new vector<float>(0);
	sample2->push_back(0);
	sample2->push_back(1);
	sample2->push_back(0.6);
	vector<float>* sample3 = new vector<float>(0);
	sample3->push_back(1);
	sample3->push_back(0);
	sample3->push_back(2.0);
	vector<float>* sample4 = new vector<float>(0);
	sample4->push_back(1);
	sample4->push_back(1);
	sample4->push_back(1.1);
	for(int i = 0; i < 10; i++)
	{
		htl->addTrainingSample(*sample1);
		htl->addTrainingSample(*sample2);
		htl->addTrainingSample(*sample3);
		htl->addTrainingSample(*sample4);
	}
	cout << "Result:\n";
	cout << htl->predict(*sample1) << "\n";
	cout << htl->predict(*sample2) << "\n";
	cout << htl->predict(*sample3) << "\n";
	cout << htl->predict(*sample4) << "\n";
	LeafNode* leaf = htl->traverseSample(*sample1);
	cout << "w0: " << leaf->linearModel->nominalWeights[0] << "\n";
	cout << "w1: " << leaf->linearModel->nominalWeights[1] << "\n";
	cout << "w2: " << leaf->linearModel->nominalWeights[2] << "\n";
	cout << "w3: " << leaf->linearModel->nominalWeights[3] << "\n";
	cout << "t: " << leaf->linearModel->threshold << "\n";
	htl->pprint();
}*/


int main() {
	//lbcpp::testPerceptronNumerical2();
	return 0;
}
