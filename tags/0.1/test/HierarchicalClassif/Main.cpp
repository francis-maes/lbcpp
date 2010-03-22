#include "GeneratedCode/HierarchicalClassif.lh"

#include <fstream>
using namespace lbcpp;


class HierarchyParser : public TextObjectParser
{
protected:
	HierarchyNode *root;
public:
	HierarchyParser(const std::string &filename,HierarchyNode *root) : TextObjectParser(filename),root(root)
	{
	}

	virtual bool parseLine(const std::string& line) 
	{
		std::vector<std::string> tab;
		tokenize(line,tab);
		
		HierarchyNode *current_node=root;
		for(int i=0;i<tab.size();i++)
		{
			int nbnode=atoi(tab[i].c_str());
			if (current_node->children.find(nbnode)==current_node->children.end())
			{
				current_node->children[nbnode]=new HierarchyNode();
				current_node->childrenIds.insert(nbnode);
				current_node->children[nbnode]->parent=current_node;
				current_node->children[nbnode]->id=nbnode;
			}
			current_node=current_node->children[nbnode];
			root->allnodes[nbnode]=current_node;
		}
		return(true);
	}
};

int cralgo()
{
std::string file_hierarchy="../../../../../data/cat_hier.txt";
  std::string file_train="../../../../../data/train.txt";
  std::string file_validation="../../../../../data/validation.txt";
//  std::string file_test="/cygdrive/c/Users/denoyer/Desktop/cralgos/data/test.txt";

  /*	
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  /*
  ** Load training classification data
  */
  ObjectStreamPtr parser_train = classificationExamplesParser(file_train, features, labels);
  if (!parser_train->isValid())
    return 1;
  
  /**
   * Charge en memoire, mais on peut apprendre en chargeant a la volée
   */
  ObjectContainerPtr trainingData = parser_train->load(100);

  HierarchyNode *hierarchy_root=new HierarchyNode();
  hierarchy_root->parent=0;
  hierarchy_root->id=-1;
  ObjectStreamPtr p=new HierarchyParser(file_hierarchy,hierarchy_root);
  p->iterate();
  std::cout << hierarchy_root->allnodes.size() << " categories found...." << std::endl;
 

  /**
  * Appel de l'algorithme
  */
  VectorObjectContainerPtr crAlgorithms = new VectorObjectContainer("CRAlgorithm");
  for (int i = 0; i < trainingData->size(); ++i)
  {
	  ClassificationExamplePtr example = trainingData->getAndCast<ClassificationExample>(i);
	  CRAlgorithmPtr crAlgo = 
			hierarchicalDescenteUniquement(hierarchy_root,example->getInput(), atoi(labels->getString(example->getOutput()).c_str()));
	  crAlgorithms->append(crAlgo);
/*	  crAlgo->run(randomPolicy());
	  VariablePtr var = crAlgo->getReturn();
	  int retour = var->getConstReference<int>();
	  std::cout << "pouet: " << i << " => " << retour << std::endl;*/
  }

	CRAlgorithmLearnerPtr learner = sarsaLearner(leastSquaresLinearRegressor(stochasticDescentLearner(constantIterationFunction(0.001)),0.01),
      0.5,
      predictedEpsilonGreedy,
      constantIterationFunction(0.01),
      maxIterationsStoppingCriterion(100));

	for (size_t i = 0; i < 10; ++i)
	{
		learner->trainStochastic((ObjectContainerPtr)crAlgorithms, consoleProgressCallback());
		PolicyStatisticsPtr statistics = new PolicyStatistics();
		learner->getPolicy()->run((ObjectContainerPtr)crAlgorithms, statistics, consoleProgressCallback());
		std::cout << "OUIAS: " << statistics->toString() << std::endl;
	}

/*  extern CRAlgorithmLearnerPtr searnLearner(RankerPtr ranker = RankerPtr(),
    ActionValueFunctionPtr optimalActionValues = ActionValueFunctionPtr(),
    double beta = 0.1,
    StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr());
*/
  
}


/**
* Baseline de maxxent multiclass  stochastique....
*/
int learning_maxent_baseline()
{
  std::string file_hierarchy="../../../../../data/cat_hier.txt";
  std::string file_train="../../../../../data/train.txt";
  std::string file_validation="../../../../../data/validation.txt";
//  std::string file_test="/cygdrive/c/Users/denoyer/Desktop/cralgos/data/test.txt";

  /*	
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  /*
  ** Load training classification data
  */
  ObjectStreamPtr parser_train = classificationExamplesParser(file_train, features, labels);
  if (!parser_train->isValid())
    return 1;
  ObjectStreamPtr parser_test = classificationExamplesParser(file_validation, features, labels);
  if (!parser_test->isValid())
    return 1;
  
  /**
   * Charge en memoire, mais on peut apprendre en chargeant a la volée
*/
  ObjectContainerPtr trainingData = parser_train->load()->randomize();
  ObjectContainerPtr testingData= parser_test->load();

  /*
  ** Create a maximum-entropy classifier (training with stochastic descent, L2 regularizer: 1)
  */
  GradientBasedLearnerPtr learner = stochasticDescentLearner(constantIterationFunction(0.01));
  //GradientBasedLearnerPtr learner = batchLearner(lbfgsOptimizer());
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels, 1);

  /*
  ** Perform training for 5 iterations
  */
  for (size_t j = 0; j < 10; ++j)
	  for (size_t i = 0; i < 5; ++i)
	  {
		classifier->trainStochastic(trainingData->fold(0, 5), consoleProgressCallback());
		std::cout << "Train RegEmpRisk: " << classifier->computeRegularizedEmpiricalRisk(trainingData->fold(0, 5)) << std::endl;
		//std::cout << "Test RegEmpRisk: " << classifier->computeRegularizedEmpiricalRisk(testingData) << std::endl;
		//  classifier->trainBatch(trainingData->randomize(), consoleProgressCallback());
/*		std::cout << "Iteration " << (i+1)
				  << " Training Accuracy: " << classifier->evaluateAccuracy(trainingData) * 100 << "%."
				  << std::endl;*/
	  }
  
  /*
  ** Parse test data and evaluate testing accuracy in one pass
  */
  std::cout << "Testing Accuracy: " << classifier->evaluateAccuracy(parser_test) * 100 << "%." << std::endl;

  /*
  ** Save the classifier
  */
//  classifier->saveToFile("classifier.model"); 

  return 0;
}

int main(int argc, char* argv[])
{
	return(cralgo());
	//return learning_maxent_baseline();
}
