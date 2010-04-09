/*-----------------------------------------.---------------------------------.
| Filename: FromScratch.lcpp               | Test Learning                   |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 16:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/Protein.lh"
#include "VariableSetModel.h"
#include "InferenceStep/ProteinInferenceSteps.h"
#include "InferenceStep/SequenceInferenceStep.h"
#include "InferenceStep/InferencePolicy.h"
using namespace lbcpp;

extern void declareProteinsClasses();
extern void declareVariableSetClasses();

////////////////////////////////
///////Inference Visitor////////
////////////////////////////////

class DefaultInferenceVisitor : public InferenceVisitor
{
public:
  virtual void visit(SequenceInferenceStepPtr inference)
  {
    for (size_t i = 0; i < inference->getNumSubSteps(); ++i)
      inference->getSubStep(i)->accept(InferenceVisitorPtr(this));
  }
};

class InferenceBuilder : public DefaultInferenceVisitor
{
public:
  
};
////////////////////////////////
//////Custom SS3 Problem////////
////////////////////////////////

// the aim is to learn a "incomplete Protein => complete Protein" function
// Input: Protein
// Output: SecondaryStructureSequence
class SS3ContentOnlyInferenceStep : public SecondaryStructureInferenceStep
{
public:
  SS3ContentOnlyInferenceStep() : SecondaryStructureInferenceStep(T("SS3ContentOnly")) {}

  virtual featureGenerator getInputFeatures(ProteinPtr protein, size_t index) const
  {
    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    ScoreVectorSequencePtr positionSpecificScores = protein->getPositionSpecificScoringMatrix();

    featureCall("a") inline aminoAcidSequence->windowFeatures(index, 8, 8, true);
    featureCall("p") inline positionSpecificScores->windowFeatures(index, 8, 8, true);
  }
};

class SS3ContentAndStructureInferenceStep : public SecondaryStructureInferenceStep
{
public:
  SS3ContentAndStructureInferenceStep(size_t passNumber)
    : SecondaryStructureInferenceStep(T("SS3ContentAndStructure") + lbcpp::toString(passNumber)) {}

  virtual featureGenerator getInputFeatures(ProteinPtr protein, size_t index) const
  {
    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    ScoreVectorSequencePtr positionSpecificScores = protein->getPositionSpecificScoringMatrix();
    SecondaryStructureSequencePtr secondaryStructure = protein->getSecondaryStructureSequence();

    featureCall("a") aminoAcidSequence->windowFeatures(index, 8, 8, true);
    featureCall("p") positionSpecificScores->windowFeatures(index, 8, 8, true);
    featureCall("w") secondaryStructure->windowFeatures(index, 5, 5, true);
  }
};

class MainInferenceStep : public SequenceInferenceStep
{
public:
  MainInferenceStep() : SequenceInferenceStep(T("Main"))
  {
    appendSubStep(new SS3ContentOnlyInferenceStep());
    appendSubStep(new SS3ContentAndStructureInferenceStep(1));
    appendSubStep(new SS3ContentAndStructureInferenceStep(2));
    appendSubStep(new SS3ContentAndStructureInferenceStep(3));
  }

  virtual ObjectPtr run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    // input and working proteins
    ProteinPtr inputProtein = input.dynamicCast<Protein>();
    jassert(inputProtein);
    ProteinPtr workingProtein = inputProtein->cloneAndCast<Protein>();
    
    // supervision
    SecondaryStructureSequencePtr correctSecondaryStructure;
    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
    if (correctProtein)
      correctSecondaryStructure = correctProtein->getSecondaryStructureSequence();

    // main inference loop
    for (size_t i = 0; i < subSteps.size(); ++i)
    {
      SecondaryStructureSequencePtr predictedSS = policy->doSubStep(subSteps[i], workingProtein, correctSecondaryStructure, returnCode);
      if (returnCode != finishedReturnCode)
        return ObjectPtr();
      workingProtein->setSecondaryStructureSequence(predictedSS);
    }

    // return the last version of the working protein
    return workingProtein;
  }
};


////////////////////////////////
//////InferencePolicy///////////
////////////////////////////////

/*
class FixedLengthChain2InferencePolicy : public DefaultInferencePolicy
{
public:
  FixedLengthChain2InferencePolicy(InferencePolicyPtr policy1, size_t policy1NumSteps, InferencePolicyPtr policy2, size_t policy2NumSteps)
    : policy1(policy1), policy1NumSteps(policy1NumSteps), policy2(policy2), policy2NumSteps(policy2NumSteps) {reset();}
  
  void reset()
    {currentPosition = 0;}

  virtual ReturnCode doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr& output)
  {
    if (currentPosition < policy1NumSteps)
    {
      ReturnCode res = policy1->doSubStep(step, input, output);
      ++currentPosition;
      return res;
    }
    else if (currentPosition < policy2NumSteps)
    {
      ReturnCode res = policy2->doSubStep(step, input, output);
      ++currentPosition;
      return res;
    }
    else
      return InferenceStep::canceledReturnCode;
  }

private:
  InferencePolicyPtr policy1;
  size_t policy1NumSteps;
  InferencePolicyPtr policy2;
  size_t policy2NumSteps;

  size_t currentPosition;
};

class LearnNthStepDeterministicInferencePolicy : public FixedLengthChain2InferencePolicy
{
public:
  LearnNthStepDeterministicInferencePolicy(size_t learnedStepNumber, InferencePolicyPtr learningPolicy)
    : FixedLengthChain2InferencePolicy(new DefaultInferencePolicy(), learnedStepNumber, learningPolicy, 1) {}
};
*/
class ExamplesCreatorPolicy : public DefaultInferencePolicy
{
public:
  ExamplesCreatorPolicy(InferencePolicyPtr explorationPolicy)
    : explorationPolicy(explorationPolicy) {}

  virtual FeatureGeneratorPtr doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input,
                                                             FeatureGeneratorPtr supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(label);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
    return explorationPolicy->doClassification(classifier, input, supervision, returnCode);
  }

protected:
  InferencePolicyPtr explorationPolicy;

  typedef std::map<LearningMachinePtr, VectorObjectContainerPtr> ExamplesMap;
  ExamplesMap examples;

  void addExample(LearningMachinePtr learningMachine, ObjectPtr example)
  {
    VectorObjectContainerPtr& machineExamples = examples[learningMachine];
    if (!machineExamples)
      machineExamples = new VectorObjectContainer();
    machineExamples->append(example);
  }
};

class GlobalSimulationLearningPolicy : public ExamplesCreatorPolicy
{
public:
  GlobalSimulationLearningPolicy()
    : ExamplesCreatorPolicy(new DefaultInferencePolicy()) {}

  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
  {
    std::cout << "Creating training examples with " << examples->size() << " episodes..." << std::endl;
    this->examples.clear();
    return examples;
  }

  virtual void supervisedExampleSetPostCallback(InferenceStepPtr inference, ObjectContainerPtr , ReturnCode& returnCode)
  {
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      machine->trainStochastic(trainingData);
      ClassifierPtr classifier = machine.dynamicCast<Classifier>();
      if (classifier)
        std::cout << "Train accuracy: " << std::flush << classifier->evaluateAccuracy(trainingData) << std::endl;
    }
  }
};

typedef ReferenceCountedObjectPtr<WholeSimulationBasedLearningPolicy> WholeSimulationBasedLearningPolicyPtr;

class ProteinEvaluationPolicy : public DecoratorInferencePolicy
{
public:
  ProteinEvaluationPolicy(InferencePolicyPtr targetPolicy)
    : DecoratorInferencePolicy(targetPolicy) {}

  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
  {
    resetEvaluation();
    return DecoratorInferencePolicy::supervisedExampleSetPreCallback(inference, examples, returnCode);
  }

  virtual ObjectPtr supervisedExamplePostCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr output, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (output)
    {
      ProteinPtr p1 = output.dynamicCast<Protein>();
      ProteinPtr p2 = supervision.dynamicCast<Protein>();
      jassert(p1 && p2);
      addProtein(p1, p2);
    }
    return DecoratorInferencePolicy::supervisedExamplePostCallback(inference, input, output, supervision, returnCode);
  }

  virtual String toString() const
  {
    double Q3dbg = correctDBG / (double)secondaryStructureAccuracy->getCount();
    String res;
    res += T("Q3: ") + String(getQ3Score() * 100.0, 2) + T("\n");
    //res += T("Q8: ") + String(getQ8Score() * 100.0, 2) + T("\n");
    return res;
  }

  double getQ3Score() const
    {return secondaryStructureAccuracy->getMean();}

  double getQ8Score() const
    {return dsspSecondaryStructureAccuracy->getMean();}

  void resetEvaluation()
  {
    correctDBG = 0;
    secondaryStructureAccuracy = new ScalarVariableMean(T("Q3"));
    dsspSecondaryStructureAccuracy = new ScalarVariableMean(T("Q8"));
  }

  void addProtein(ProteinPtr predicted, ProteinPtr correct)
  {
    addLabelSequence(predicted->getSecondaryStructureSequence(), correct->getSecondaryStructureSequence(), secondaryStructureAccuracy);
    addLabelSequence(predicted->getSecondaryStructureSequence(true), correct->getSecondaryStructureSequence(true), dsspSecondaryStructureAccuracy);
  }

  void addLabelSequence(LabelSequencePtr predicted, LabelSequencePtr correct, ScalarVariableMeanPtr statistics)
  {
    if (!correct)
      return;
    jassert(predicted);
    size_t n = predicted->getLength();
    jassert(correct->getLength() == n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t correctLabel;
      size_t predictedLabel;
      if (correct->getVariable(i, correctLabel))
      {
        if ((predicted->getVariable(i, predictedLabel) && predictedLabel == correctLabel))
          ++correctDBG;
        statistics->push((predicted->getVariable(i, predictedLabel) && predictedLabel == correctLabel) ? 1.0 : 0.0);
      }
    }
  }

protected:
  ScalarVariableMeanPtr secondaryStructureAccuracy;
  ScalarVariableMeanPtr dsspSecondaryStructureAccuracy;
  size_t correctDBG;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationPolicy> ProteinEvaluationPolicyPtr;

////////////////////////////////
/////////MAIN///////////////////
////////////////////////////////


int main(int argc, char** argv)
{
  declareProteinsClasses();
  declareVariableSetClasses();


  File modelFile(T("C:\\Projets\\Proteins\\data\\models\\test.model"));
  File proteinsDirectory(T("C:\\Projets\\Proteins\\data\\CB513cool"));
  ObjectStreamPtr proteinsStream = directoryObjectStream(proteinsDirectory, T("*.protein"));
  ObjectContainerPtr proteins = proteinsStream->load()->randomize();

  ObjectContainerPtr trainingData = proteins->invFold(0, 7);
  ObjectContainerPtr testingData = proteins->fold(0, 7);
  std::cout << trainingData->size() << " Training Proteins "
            << testingData->size() << " Testing Proteins" << std::endl;

  InferenceStepPtr mainInferenceStep = new MainInferenceStep();
  InferencePolicyPtr inferencePolicy = new GlobalSimulationLearningPolicy();
  for (size_t i = 0; i < 100; ++i)
  {
    std::cout << std::endl << " ================== ITERATION " << i << " ================== " << std::endl;
    ProteinEvaluationPolicyPtr onlineEvaluation = new ProteinEvaluationPolicy(inferencePolicy);
    onlineEvaluation->runOnSelfSupervisedExampleSet(mainInferenceStep, trainingData);
    std::cout << "Online evaluation:" << std::endl << onlineEvaluation->toString() << std::endl;

    onlineEvaluation = new ProteinEvaluationPolicy(new DefaultInferencePolicy());
    onlineEvaluation->runOnSelfSupervisedExampleSet(mainInferenceStep, trainingData);
    std::cout << "Train evaluation:" << std::endl << onlineEvaluation->toString() << std::endl;

    onlineEvaluation->runOnSelfSupervisedExampleSet(mainInferenceStep, testingData);
    std::cout << "Test evaluation:" << std::endl << onlineEvaluation->toString() << std::endl;
  }

//  ObjectGraphPtr dependencyGraph = mainInferenceStep->getDependencyGraph();


//  MySupervisedTrainingCallback callback;
//  trainSupervised(mainInferenceStep, trainingData, callback);
  
  return 0;
}
