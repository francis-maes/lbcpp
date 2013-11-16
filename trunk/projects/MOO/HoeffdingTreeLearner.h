#ifndef HOEFFDINGTREELEARNER_H
#define HOEFFDINGTREELEARNER_H

using namespace std;

#include <vector>
#include <stdlib.h> // rand
#include <cmath>
#include <iomanip> // std::setw
#include <oil/Execution/ExecutionContext.h>

class MathUtils;
class DerivedModel;
class EBST;
class Node;
class LeafNode;
class InternalNode;
class AttributeObservation;
class Perceptron;
class Split;

class MathUtils {
public:
	static double Log2( double n )  
	{  
		// log(n)/log(2) is log2.  
		return log( n ) / log( 2.0f );  
	}
	// incremental mean
	static double mean(double Sy, unsigned N){
		return Sy / N;
	}
	// incremental standard deviation
	static double sd(double Sy, double Syy, unsigned N){
		return sqrt((Syy - (Sy*Sy)/N)/N);
	}

	// incremental standard deviation reduction
	static double sdr(double sdP, double sdL, double sdR, int nL, int nR){
		return sdP - (nL*sdL + nR*sdR) / (nL + nR);
	}

	static double hoeffdingBound(unsigned R, unsigned N, double delta){
		return (N==0 || delta==0)?1:sqrt(R*R*Log2(1/delta)/2/N);
	}

	static double randDouble(){
		return (double)rand()/(double)RAND_MAX;
	}

	static double normalize(double y, double mean, double sd){
		return sd == 0? (y - mean) : (y - mean) / (3*sd);
	}
};

class AttributeDefinition {
public:
	string name;
	int index;

	double Sx, Sxx;
	int examplesSeen;

	AttributeDefinition(int index, string name){
		this->index = index;
		this->name = name;
		this->Sx = 0;
		this->Sxx = 0;
		this->examplesSeen = 0;
	}

	double getMean() const{
		return MathUtils::mean(Sx,examplesSeen);
	}

	double getStd() const{
		return MathUtils::sd(Sx, Sxx, examplesSeen);
	}

	void addAttributeInstance(const double x){
		Sx+=x;
		Sxx+=x*x;
		examplesSeen++;
	}
};

class NominalAttributeDefinition : public AttributeDefinition{
public:
	unsigned size;

	NominalAttributeDefinition(int index, string name, unsigned size) : AttributeDefinition(index, name){
		this->size = size;
	}
};

class NumericalAttributeDefinition : public AttributeDefinition{
public:
	NumericalAttributeDefinition(int index, string name) : AttributeDefinition(index, name){
	}
};

// assumes the last attribute is the target attribute
class DataDefinition {
public:
	vector<NominalAttributeDefinition> nominalAttributeDefinitions;
	vector<NumericalAttributeDefinition> numericalAttributeDefinitions;
	vector<AttributeDefinition> attributeDefinitions;
	int nbAttributes;

	DataDefinition(){
		nominalAttributeDefinitions = vector<NominalAttributeDefinition>();
		numericalAttributeDefinitions = vector<NumericalAttributeDefinition>();
		attributeDefinitions = vector<AttributeDefinition>();
		nbAttributes = 0;
	}

	void addNominalAttribute(string name, unsigned size){
		NominalAttributeDefinition def = NominalAttributeDefinition(nbAttributes, name, size);
		nominalAttributeDefinitions.push_back(def);
		attributeDefinitions.push_back(def);
		nbAttributes++;
	}

	void addNumericalAttribute(string name){
		NumericalAttributeDefinition def = NumericalAttributeDefinition(nbAttributes, name);
		numericalAttributeDefinitions.push_back(def);
		attributeDefinitions.push_back(def);
		nbAttributes++;
	}

	void addSample(const vector<float>& sample){
		int index;
		for(unsigned i = 0; i < nominalAttributeDefinitions.size(); i++){
			if(i == 0){
				index = sample[nominalAttributeDefinitions[0].index];
			}
			nominalAttributeDefinitions[i].addAttributeInstance(sample[index]);
			index +=-sample[nominalAttributeDefinitions[i].index] +
					nominalAttributeDefinitions[i].size +
					sample[nominalAttributeDefinitions[i+1].index];
		}
		for(unsigned i = 0; i < numericalAttributeDefinitions.size(); i++){
			numericalAttributeDefinitions[i].addAttributeInstance(sample[numericalAttributeDefinitions[i].index]);
		}
	}

	// TODO: temporary measure
	void addTargetAttribute(string name){
		nbAttributes++;
	}
};

// the last weight contains the threshold
class Perceptron {
public:
	vector<double> nominalWeights;
	vector<double> numericalWeights;
	double threshold;
	double learningRate;
	double initialLearningRate;
	double learningRateDecay;
	const DataDefinition* dataDefinition;

	Perceptron(const DataDefinition& dataDefinition, double initialLearningRate, double learningRateDecay){
		// initialize nominal weights
		int totalNominalSize = 0;
		for(unsigned i = 0; i < dataDefinition.nominalAttributeDefinitions.size(); i++)
			totalNominalSize += dataDefinition.nominalAttributeDefinitions[i].size;
		nominalWeights.resize(totalNominalSize);
		for(int i = 0; i < totalNominalSize; i++)
			nominalWeights[i] = MathUtils::randDouble()*2 - 1;
		// initialize numerical weights
		numericalWeights.resize(dataDefinition.numericalAttributeDefinitions.size());
		for(unsigned i = 0; i < dataDefinition.numericalAttributeDefinitions.size(); i++)
			numericalWeights[i] = MathUtils::randDouble()*2 - 1;
		threshold = MathUtils::randDouble()*2 - 1;
		learningRate = initialLearningRate;
		this->initialLearningRate = initialLearningRate;
		this->learningRateDecay = learningRateDecay;
		this->dataDefinition = &dataDefinition;
	}

	Perceptron(const Perceptron& perceptron){
		this->nominalWeights = vector<double>(perceptron.nominalWeights);
		this->numericalWeights = vector<double>(perceptron.numericalWeights);
		this->threshold = perceptron.threshold;
		this->learningRate = perceptron.initialLearningRate;
		this->initialLearningRate = perceptron.initialLearningRate;
		this->learningRateDecay = perceptron.learningRateDecay;
		this->dataDefinition = perceptron.dataDefinition;
	}

	void train(const vector<float>& sample){
		unsigned n = 1; // Number of samples used
		learningRate = initialLearningRate / (1 + n * learningRateDecay);
		// update nominal weights
		double dy = sample[sample.size() - 1] - predict(sample);
		int index;
		for(unsigned i = 0; i < dataDefinition->nominalAttributeDefinitions.size(); i++){
			if(i == 0){
				index = sample[dataDefinition->nominalAttributeDefinitions[0].index];
			}
			nominalWeights[index] += learningRate * dy * 1;
			index +=-sample[(dataDefinition->nominalAttributeDefinitions)[i].index] +
					(dataDefinition->nominalAttributeDefinitions)[i].size +
					sample[(dataDefinition->nominalAttributeDefinitions)[i+1].index];
		}
		// update numerical weights
		for(unsigned i = 0; i < dataDefinition->numericalAttributeDefinitions.size(); i++){
			cout << "------------------------" << numericalWeights.size() << endl;
			cout << "------------------------" << numericalWeights[i] << endl;
			numericalWeights[i] += learningRate * dy * sample[dataDefinition->numericalAttributeDefinitions[i].index];
			cout << "------------------------" << sample[dataDefinition->numericalAttributeDefinitions[i].index] << endl;
			cout << "------------------------" << dataDefinition->numericalAttributeDefinitions[i].index << endl;
			cout << "------------------------" << numericalWeights[i] << endl;
		}
		// update threshold
		threshold += learningRate * dy * 1;
	}

	double predict(const vector<float>& sample){
		double prediction = 0;
		// use nominal weights
		int index;
		double normalizedX;
		for(unsigned i = 0; i < dataDefinition->nominalAttributeDefinitions.size(); i++){
			if(i == 0){
				index = sample[dataDefinition->nominalAttributeDefinitions[0].index];
			}
			prediction += nominalWeights[index] * 1;
			index +=-sample[dataDefinition->nominalAttributeDefinitions[i].index] +
					dataDefinition->nominalAttributeDefinitions[i].size +
					sample[dataDefinition->nominalAttributeDefinitions[i+1].index];
		}
		// use numerical weights
		for(unsigned i = 0; i < dataDefinition->numericalAttributeDefinitions.size(); i++){
			prediction += numericalWeights[i] * sample[dataDefinition->numericalAttributeDefinitions[i].index];
		}
		// use threshold
		prediction += threshold * 1;
		return prediction;
	}
};

class Split {
private:
  void init(unsigned attributeNb, double value, double quality) {
		this->attributeNb = attributeNb;
		this->value = value;
		this->quality = quality;
  }
public:
	double quality;
	unsigned attributeNb;
	double value;

	Split(unsigned attributeNb, double value, double quality) {
    init(attributeNb, value, quality);
	}

	Split() {
    init(0, 0, 0);
  }

	int Compare (const Split& split) const {

		if(quality < split.quality)
			return -1;
		else if(quality == split.quality)
			return 0;
		else
			return 1;
	}

	bool operator < (const Split& split) const {
	  return Compare(split)<0;
	}
};

class DerivedModel {
public:
	unsigned n; /* number of samples */
	double Sy; /* sum of y values */
	double Syy; /* sum of y-squared values */

	DerivedModel(){
		n = 0;
		Sy = 0;
		Syy = 0;
	};

	void update(int dn, double dSy, double dSyy){
		n += dn;
		Sy += dSy;
		Syy += dSyy;
	}
};

//extended binary search tree
class EBST {
public:
	EBST* left;
	EBST* right;

	DerivedModel* leftModel;
	DerivedModel* rightModel;

	double key; // each node has a value

	EBST() {
		left = NULL;
		right = NULL;

		leftModel = new DerivedModel();
		rightModel = new DerivedModel();

		key = 0;
	}

	void add(double attribute, double y) {
		if(isEmpty() || attribute == key){
			leftModel->update(1,y,y*y);
			key = attribute;
		}
		else if(attribute < key){
			if(!hasLeftChild()) {
				left = new EBST();
			}
			leftModel->update(1,y,y*y);
			left->add(attribute, y);
		}
		else {
			if(!hasRightChild()) {
				right = new EBST();
			}
			rightModel->update(1,y,y*y);
			right->add(attribute, y);
		}

	}

	void findBestSplit(double& maxSDR, double& splitPoint) const{
		DerivedModel * totalLeftModel = new DerivedModel();
		DerivedModel * totalRightModel = new DerivedModel();
		totalLeftModel->Sy = 0;
		totalRightModel->Sy = leftModel->Sy + rightModel->Sy;
		totalLeftModel->Syy = 0;
		totalRightModel->Syy = leftModel->Syy + rightModel->Syy;
		totalRightModel->n = leftModel->n + rightModel->n;
		double total = leftModel->n + rightModel->n;
		maxSDR = 0;
		findBestSplit(*this, maxSDR, splitPoint, *totalLeftModel, *totalRightModel, total);
	}

private:
	void findBestSplit(const EBST& node, double& maxSDR, double& splitPoint,
		DerivedModel& totalLeftModel, DerivedModel& totalRightModel, double& total) const{
		if(node.hasLeftChild()){
			findBestSplit(*node.left, maxSDR, splitPoint,
					totalLeftModel, totalRightModel, total);
		}
		//update the sums and counts for computing the SDR of the split
		totalLeftModel.update(0, node.leftModel->Sy, node.leftModel->Syy);
		totalRightModel.update(-node.leftModel->n, -node.leftModel->Sy, -node.leftModel->Syy);
		double sdParent = MathUtils::sd(totalLeftModel.Sy+totalRightModel.Sy, totalLeftModel.Syy+totalRightModel.Syy, total);
		double sdLeftChild = MathUtils::sd(totalLeftModel.Sy, totalLeftModel.Syy, total - totalRightModel.n);
		double sdRightChild = MathUtils::sd(totalRightModel.Sy, totalRightModel.Syy, totalRightModel.n);
		double sdr = MathUtils::sdr(sdParent, sdLeftChild, sdRightChild, total - totalRightModel.n, totalRightModel.n);
		if(maxSDR < sdr){
			//cout <<"new max:" << maxSDR << "\n";
			maxSDR = sdr;
			splitPoint = node.key;
		}
		if(node.hasRightChild()){
			findBestSplit(*node.right, maxSDR, splitPoint,
					totalLeftModel, totalRightModel, total);
		}
		//update the sums and counts for returning to the parent node
		totalLeftModel.update(0, -node.leftModel->Sy, -node.leftModel->Syy);
		totalRightModel.update(node.leftModel->n, node.leftModel->Sy, node.leftModel->Syy);
	}

	bool isEmpty() const{
		return leftModel->n == 0 && rightModel->n == 0;
	}

	bool hasLeftChild() const{
		return left != NULL;
	}

	bool hasRightChild() const{
		return right != NULL;
	}
};

class AttributeObservation {
public:
	unsigned attributeNb;

	virtual ~AttributeObservation(){};
	AttributeObservation(unsigned attributeNb) {
		this->attributeNb = attributeNb;
	};
	virtual Split* findBestSplitPoint() = 0;
};

class NominalAttributeObservation : public AttributeObservation {
private:
  void init(unsigned nbNominalValues) {
		models.resize(nbNominalValues);
		for(unsigned i = 0; i < nbNominalValues; i++)
			models[i] = *(new DerivedModel());
  }
public:
	vector<DerivedModel> models;

	~NominalAttributeObservation() {};

	NominalAttributeObservation(unsigned attributeNb, unsigned nbNominalValues) : AttributeObservation(attributeNb) {
    init(nbNominalValues);
	}

	NominalAttributeObservation() : AttributeObservation(-1) {
    init(0);
  }

	Split* findBestSplitPoint() {
		Split* bestSplit = new Split(attributeNb, 0, 0);
		DerivedModel* totalModel = new DerivedModel();
		for(unsigned k = 0; k < models.size(); k++){
			DerivedModel m = models[k];
			totalModel->update(m.n, m.Sy, m.Syy);
		}
		for(unsigned j = 0; j < models.size(); j++){
			double splitQuality = 0;
			DerivedModel m = models[j];
			double sdP = MathUtils::sd(totalModel->Sy, totalModel->Syy, totalModel->n);
			double sdL = MathUtils::sd(m.Sy, m.Syy, m.n);
			double sdR = MathUtils::sd(totalModel->Sy-m.Sy, totalModel->Syy-m.Syy, totalModel->n-m.n);
			splitQuality = MathUtils::sdr(sdP, sdL, sdR, m.n, totalModel->n-m.n);
			if(splitQuality > bestSplit->quality) {
				delete bestSplit;
				bestSplit->quality = splitQuality;
				bestSplit->value = j;
			}
		}
		return bestSplit;
	}
};

class NumericalAttributeObservation : public AttributeObservation {
private:
  void init() {
		model = new EBST();
  }
  
public:
	EBST* model;

	~NumericalAttributeObservation() {};

	NumericalAttributeObservation(unsigned attributeNb) : AttributeObservation(attributeNb){
    init();
	}

	NumericalAttributeObservation() : AttributeObservation(-1) {
    init();
  }

	Split* findBestSplitPoint() {
		double maxSDR;
		double splitPoint;
		model->findBestSplit(maxSDR, splitPoint);
		return new Split(attributeNb, splitPoint, maxSDR);
	}
};

class Node {
public:
	InternalNode* parent;
	virtual ~Node() {};

	Node(){
		parent = NULL;
	};

	Node(InternalNode* parent){
		this->parent = parent;
	}

	virtual bool isLeaf() const = 0;
	virtual void pprint(int indent = 0) const = 0;
	virtual LeafNode* traverseSample(const vector<float>& sample) = 0;

	bool isRoot() const{
		return parent == NULL;
	}
};

class LeafNode : public Node {
public:
	const DataDefinition* dataDefinition;
	Perceptron* linearModel;
	vector<NominalAttributeObservation> nominalAttributeObservations;
	vector<NumericalAttributeObservation> numericalAttributeObservations;
	unsigned examplesSeen;


	~LeafNode() {};

	LeafNode(const DataDefinition& dataDefinition, double initialLearningRate, double learningRateDecay, InternalNode* parent) : Node(parent){
		this->dataDefinition = &dataDefinition;
		examplesSeen = 0;
		linearModel = new Perceptron(dataDefinition, initialLearningRate, learningRateDecay);
		initAttributeObservations();
	}

	LeafNode(const DataDefinition& dataDefinition, const Perceptron& linearModel, InternalNode* parent) : Node(parent){
		this->dataDefinition = &dataDefinition;
		examplesSeen = 0;
		this->linearModel = new Perceptron(linearModel);
		initAttributeObservations();
	}

	bool isLeaf() const{
		return true;
	}

	void pprint(int indent = 0) const {
        if (indent) cout << setw(indent) << ' ';
		cout<< "samples: " << examplesSeen << endl;
	}

	LeafNode* traverseSample(const vector<float>& sample) {
		return this;
	}

private:
	void initAttributeObservations() {
		nominalAttributeObservations = vector<NominalAttributeObservation>(dataDefinition->nominalAttributeDefinitions.size());
		for(unsigned i = 0; i < dataDefinition->nominalAttributeDefinitions.size(); i++){
			nominalAttributeObservations[i] = NominalAttributeObservation(dataDefinition->nominalAttributeDefinitions[i].index, dataDefinition->nominalAttributeDefinitions[i].size);
		}

		numericalAttributeObservations = vector<NumericalAttributeObservation>(dataDefinition->numericalAttributeDefinitions.size());
		for(unsigned i = 0; i < dataDefinition->numericalAttributeDefinitions.size(); i++){
			numericalAttributeObservations[i] = NumericalAttributeObservation(dataDefinition->numericalAttributeDefinitions[i].index);
		}
	}
};

class InternalNode : public Node {
public:
	Node* left;
	Node* right;

	const Split* split;

	~InternalNode() {};

	InternalNode(Split* split, LeafNode* left, LeafNode* right, InternalNode* parent){
		this->left = left;
		this->right = right;
		this->parent = parent;
		this->split = split;
	}

	bool isLeaf() const{
		return false;
	}

	void pprint(int indent) const{
		if (indent) cout << setw(indent) << ' ';
		cout<< split->attributeNb << "/" << split->value << endl;
		cout << "left:" << endl;
		left->pprint(indent+4);
		cout << "right:" << endl;
		right->pprint(indent+4);
	}

	LeafNode* traverseSample(const vector<float>& sample) {
		if(sample[split->attributeNb] <= split->value){
			return left->traverseSample(sample);
		}
		else{
			return right->traverseSample(sample);
		}
	}

};

// nominal values go from 0 to nbNominalValues
// sample has n - 1 attributes and 1 target at index n -1
class HoeffdingTreeLearner {

protected:
	double delta; /* confidence level */
	unsigned chunkSize; /* number of samples before tree is recalculated */
	bool pruneOnly; /* whether to prune only or to generate alternate trees for drift detection */
	DataDefinition* dataDefinition; /* the data definition of the samples */
	double initialLearningRate; /* initial learning rate for the perceptron */
	double learningRateDecay; /* decay of learning rate for the perceptron */
	double threshold; /* threshold for splitting criterium */
	int verbosity;
	Node* root;
	lbcpp::ExecutionContext& context;

	unsigned seenExamples; /* number of unprocessed examples */
public:
	HoeffdingTreeLearner(lbcpp::ExecutionContext& context, double delta, DataDefinition& dataDefinition);
	HoeffdingTreeLearner();
	~HoeffdingTreeLearner();
	/* add the training sample to the tree */
	void addTrainingSample(const vector<float>& sample);
	double predict(const vector<float>& sample) const;
	void pprint() const;
	/* traverses the sample to the leaf of the tree */
	LeafNode* traverseSample(const vector<float>& sample) const; // temp for testing, should be private
	vector<Split>* findBestSplitPerAttribute(const LeafNode& leaf) const;
	void normalizeSample(vector<float>& sample, LeafNode& leaf) const;
private:
	void updateStatistics(const vector<float>& sample, LeafNode& leaf);
	void updateLinearModel(const vector<float>& sample, LeafNode& leaf);
	InternalNode* makeSplit(const Split& split, const LeafNode& leaf);
	void swap(Node& originalNode, Node& newNode);
	double getBestSplitratio(const std::vector<Split>& bestSplits) const;
};


#endif
