/*-----------------------------------------.---------------------------------.
| Filename: TestLearning.cpp               | Test Learning                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2010 12:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/Protein.lh"
using namespace lbcpp;

extern void declareProteinsClasses();

class InterdependantVariableSetScoreFunction : public Object
{
public:
  virtual void reset() = 0;
  virtual void addPrediction(InterdependantVariableSetPtr prediction, InterdependantVariableSetPtr correct) = 0;
  virtual double compute() const = 0;
};

class LabelAccuracyVariableSetScoreFunction : public InterdependantVariableSetScoreFunction
{
public:
  LabelAccuracyVariableSetScoreFunction()
    {reset();}

  virtual void reset()
    {numCorrect = numVariables = 0;}

  virtual void addPrediction(InterdependantVariableSetPtr prediction, InterdependantVariableSetPtr correct)
  {
    size_t n = prediction->getNumVariables();
    jassert(n == correct->getNumVariables());
    for (size_t i = 0; i < n; ++i)
    {
      size_t correctLabel;
      if (correct->getVariable(i, correctLabel))
      {
        ++numVariables;
        size_t predictedLabel;
        if (prediction->getVariable(i, predictedLabel) && predictedLabel == correctLabel)
          ++numCorrect;
      }
    }
  }
  
  virtual double compute() const
    {return numVariables ? numCorrect / (double)numVariables : 0.0;}

private:
  size_t numCorrect;
  size_t numVariables;
};

class InterdependantVariableSetModel : public LearningMachine
{
public:
  virtual void predict(VariableFeatureFunctionPtr inputFeatures, InterdependantVariableSetPtr prediction) = 0;
};

typedef ReferenceCountedObjectPtr<InterdependantVariableSetModel> InterdependantVariableSetModelPtr;

class ClassifierBasedInterdependantVariableSetModel : public InterdependantVariableSetModel
{
public:
  ClassifierBasedInterdependantVariableSetModel(ClassifierPtr classifier = ClassifierPtr())
    : classifier(classifier) {}

  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    VectorObjectContainerPtr classificationExamples = new VectorObjectContainer("ClassificationExample");
    for (size_t i = 0; i < examples->size(); ++i)
    {
      InterdependantVariableSetExamplePtr example = examples->getAndCast<InterdependantVariableSetExample>(i);
      jassert(example);
      example->createClassificationExamples(lbcpp::vectorObjectContainerFiller(classificationExamples));
      std::cout << "Num examples: " << classificationExamples->size() << std::endl;
    }
    return classifier->trainBatch((ObjectContainerPtr)classificationExamples, progress);
  }

  virtual void predict(VariableFeatureFunctionPtr inputFeatures, InterdependantVariableSetPtr prediction)
  {
    for (size_t i = 0; i < prediction->getNumVariables(); ++i)
      prediction->setVariable(i, classifier->predict(inputFeatures->computeFeatures(i)));
  }

protected:
  ClassifierPtr classifier;
};

void declareInterdependantVariableSetClasses()
{
  LBCPP_DECLARE_CLASS(ClassifierBasedInterdependantVariableSetModel);
}

class ProteinToInterdependantVariableSetExample : public ObjectFunction
{
public:
  virtual String getOutputClassName(const String& inputClassName) const
    {return T("InterdependantVariableSetExample");}

  virtual ObjectPtr function(ObjectPtr object) const
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    
    SequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    SequencePtr positionSpecificScores = protein->getPositionSpecificScoringMatrix();
    SequencePtr secondaryStructure3 = protein->getSecondaryStructureSequence(false);
    SequencePtr secondaryStructure8 = protein->getSecondaryStructureSequence(true);

    VariableFeatureFunctionPtr aminoAcid = new SequenceElementFeatureFunction(aminoAcidSequence);
    VariableFeatureFunctionPtr aminoAcidWindow = new WindowVariableFeatureFunction(aminoAcid, 15, 15, true);

    VariableFeatureFunctionPtr pssmColumn = new SequenceElementFeatureFunction(positionSpecificScores);
    VariableFeatureFunctionPtr pssmWindow = new WindowVariableFeatureFunction(pssmColumn, 15, 15, true);

    VariableFeatureFunctionPtr secondaryStructureCorrectWindow
        = new WindowVariableFeatureFunction(new ContentOnlyVariableFeatureFunction(secondaryStructure8), 10, 10, false);

    VariableFeatureFunctionPtr inputFeatures
      = new UnionVariableFeatureFunction(aminoAcidWindow, pssmWindow, secondaryStructureCorrectWindow);
    
    return new InterdependantVariableSetExample(inputFeatures, secondaryStructure3);
  }
};

int main()
{
  declareProteinsClasses();
  ObjectStreamPtr proteinsStream = directoryObjectStream(File(T("C:/Projets/Proteins/data/CB513cool")), T("*.protein"));
  ObjectStreamPtr examplesStream = proteinsStream->apply(new ProteinToInterdependantVariableSetExample());
  ObjectContainerPtr examples = examplesStream->load()->randomize();
  StringDictionaryPtr variablesDictionary = examples->getAndCast<InterdependantVariableSetExample>(0)->getTargetVariables()->getVariablesDictionary();

  size_t numFolds = 7;
  for (size_t i = 0; i < numFolds; ++i)
  {
    IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);//->stochasticToBatchLearner();
    GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, variablesDictionary);
    classifier->setL2Regularizer(0.001);
    InterdependantVariableSetModelPtr model = new ClassifierBasedInterdependantVariableSetModel(classifier);

    model->trainBatch(examples->invFold(i, numFolds), consoleProgressCallback());
    //model->evaluate(examples->fold(i, numFolds));
  }
  /*
  // create classification dataset
  StringDictionaryPtr outputLabels;
  VectorObjectContainerPtr classificationExamples = new VectorObjectContainer("ClassificationExample");
  while (!examplesStream->isExhausted())
  {
    InterdependantVariableSetExamplePtr example = examplesStream->nextAndCast<InterdependantVariableSetExample>();
    jassert(example);
    outputLabels = example->getTargetVariables()->getVariablesDictionary();
    example->createClassificationExamples(lbcpp::vectorObjectContainerFiller(classificationExamples));
    std::cout << "Num examples: " << classificationExamples->size() << std::endl;
  }

  // create learning machine
  IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
  GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, outputLabels);
  classifier->setL2Regularizer(0.001);

  // stochastic training
  std::cout << "Training ..." << std::endl;
  for (size_t i = 0; i < 5; ++i)
  {
    classifier->trainStochastic(classificationExamples->randomize());
    std::cout << "Iteration " << (i+1)
              << " Training Accuracy: " << classifier->evaluateAccuracy((ObjectContainerPtr)classificationExamples) * 100 << "%."
              << std::endl;
  }*/

  // OBJECTIF1: LANCER CO et OPT sur PSSM->SS3 avec differentes tailles de fenetres en entrée et en sortie
  // OBJECTIF2: LANCER SICA sur PSSM->SS3 
  // OBJECTIF3: LANCER CO et OPT sur AA->PSSM

  // Results: Predition of SS3:
  // AA(15) => 62.2
  // PSSM(15) => 72.1
  // AA(15)+PSSM(15) => 73.9
  // AA(15)+PSSM(15)+OPT(1) => 88.3
  // AA(15)+PSSM(15)+OPT(10) => 89.1
  // AA(15)+PSSM(15)+OPT_8(10) => 90.5
  // PSSM(15)+OPT(1) => 87.8
  // AA(15)+OPT(1) => 87.6
  // OPT(1) => 82.8
  // OPT(10) => 84.7
  return 0;
}
