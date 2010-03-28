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

class InterdependantVariableSetModel : public LearningMachine
{
public:
  
};

typedef ReferenceCountedObjectPtr<InterdependantVariableSetModel> InterdependantVariableSetModelPtr;

class ClassifierBasedInterdependantVariableSetModel : public InterdependantVariableSetModel
{
public:
  virtual ClassifierPtr createClassifier(StringDictionaryPtr labels)
  {
    IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);
    GradientBasedClassifierPtr res = maximumEntropyClassifier(/*learner->stochasticToBatch(100)*/learner, labels);
    //res->setL2Regularizer(l2regularizer); // FIXME: l2regularizer
    return res;
  }

  virtual void trainStochasticBegin()
  {
    if (!classifier)
      classifier = createClassifier(StringDictionaryPtr()); // FIXME: labels 
    classifier->trainStochasticBegin();
  }

  virtual void trainStochasticEnd()
    {classifier->trainStochasticEnd();}

  virtual void trainStochasticExample(ObjectPtr example)
  {
    InterdependantVariableSetPtr variables = example.dynamicCast<InterdependantVariableSet>();
    jassert(variables->getVariablesType() == InterdependantVariableSet::discreteVariable);
    size_t numVariables = variables->getNumVariables();

    for (size_t i = 0; i < numVariables; ++i)
    {
      size_t label;
      if (variables->getVariable(i, label))
        classifier->trainStochasticExample(new ClassificationExample(featureFunction->computeFeatures(i), label));
    }
  }

protected:
  ClassifierPtr classifier;
  VariableFeatureFunctionPtr featureFunction;
};

void declareInterdependantVariableSetClasses()
{
  LBCPP_DECLARE_CLASS(ClassifierBasedInterdependantVariableSetModel);
}

class InterdependantVariableSetExample : public LearningExample
{
public:
  InterdependantVariableSetExample(VariableFeatureFunctionPtr inputFeatures, InterdependantVariableSetPtr targetVariables)
    : inputFeatures(inputFeatures), targetVariables(targetVariables) {}

  void createClassificationExamples(ObjectConsumerPtr target)
  {
    jassert(inputFeatures->getNumVariables() == targetVariables->getNumVariables());
    jassert(targetVariables->getVariablesType() == InterdependantVariableSet::discreteVariable);

    size_t n = targetVariables->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      size_t value;
      if (targetVariables->getVariable(i, value))
        target->consume(new ClassificationExample(inputFeatures->computeFeatures(i)->toSparseVector(), value));
    }
  }

  InterdependantVariableSetPtr getTargetVariables() const
    {return targetVariables;}

private:
  VariableFeatureFunctionPtr inputFeatures;
  InterdependantVariableSetPtr targetVariables;
};

typedef ReferenceCountedObjectPtr<InterdependantVariableSetExample> InterdependantVariableSetExamplePtr;

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
  }

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
