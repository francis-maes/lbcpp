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

class InterdependantVariableSetFeatureFunction : public Object
{
public:
  virtual FeatureGeneratorPtr computeFeatures(InterdependantVariableSetPtr variables, size_t variableIndex) const = 0; 
};

typedef ReferenceCountedObjectPtr<InterdependantVariableSetFeatureFunction>
  InterdependantVariableSetFeatureFunctionPtr;

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
        classifier->trainStochasticExample(new ClassificationExample(featureFunction->computeFeatures(variables, i), label));
    }
  }

protected:
  ClassifierPtr classifier;
  InterdependantVariableSetFeatureFunctionPtr featureFunction;
};

void declareInterdependantVariableSetClasses()
{
  LBCPP_DECLARE_CLASS(ClassifierBasedInterdependantVariableSetModel);
}

class DirectoryBasedObjectStream : public ObjectStream
{
public:

};


int main()
{
  declareProteinsClasses();

  // OBJECTIF1: LANCER CO et OPT sur PSSM->SS3 avec differentes tailles de fenetres en entrée et en sortie
  // OBJECTIF2: LANCER SICA sur PSSM->SS3 
  // OBJECTIF3: LANCER CO et OPT sur AA->PSSM

  return 0;
}
