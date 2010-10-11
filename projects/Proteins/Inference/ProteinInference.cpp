/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.cpp           | Protein Top Level Inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 01:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinInference.h"
#include <lbcpp/Perception/Perception.h>
using namespace lbcpp;

/*
** ProteinInferenceHelper
*/
void ProteinInferenceHelper::setPDBDebugDirectory(const File& directory)
{
  pdbDebugDirectory = directory;
  if (!directory.exists())
    directory.createDirectory();
}
  
void ProteinInferenceHelper::setProteinDebugDirectory(const File& directory)
{
  proteinDebugDirectory = directory;
  if (!directory.exists())
    directory.createDirectory();
}

ProteinPtr ProteinInferenceHelper::prepareInputProtein(const Variable& input)
{
  const ProteinPtr& inputProtein = input.getObjectAndCast<Protein>();
  jassert(inputProtein);
  ProteinPtr workingProtein = inputProtein->cloneAndCast<Protein>();
  jassert(workingProtein);
  return workingProtein;
}

void ProteinInferenceHelper::prepareSupervisionProtein(ProteinPtr protein)
{
  jassert(protein);
  protein->computeMissingVariables();
  if (pdbDebugDirectory.exists() && protein->getTertiaryStructure())
    protein->saveToPDBFile(pdbDebugDirectory.getChildFile(protein->getName() + T("_correct.pdb")));
  if (proteinDebugDirectory.exists())
    protein->saveToXmlFile(proteinDebugDirectory.getChildFile(protein->getName() + T("_correct.xml")));
}

void ProteinInferenceHelper::saveDebugFiles(ProteinPtr protein, size_t stepNumber)
{
  String idx((int)stepNumber);

   if (pdbDebugDirectory.exists() &&  protein->getTertiaryStructure())
    protein->saveToPDBFile(pdbDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".pdb")));

  if (proteinDebugDirectory.exists())
    protein->saveToXmlFile(proteinDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".xml")));
}

/*
** ProteinSequentialInference
*/
ProteinSequentialInference::ProteinSequentialInference()
  : VectorSequentialInference(T("Protein"))
{
}

SequentialInferenceStatePtr ProteinSequentialInference::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  if (supervision.exists())
    prepareSupervisionProtein(supervision.getObjectAndCast<Protein>());
  return VectorSequentialInference::prepareInference(context, prepareInputProtein(input), supervision, returnCode);
}

void ProteinSequentialInference::prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
{
  // we keep the same input and supervision for sub-inferences
  Variable inputProtein;
  if (index > 0)
    inputProtein = state->getSubOutput(); // take the last version of the working protein
  else
    inputProtein = state->getInput();
  jassert(inputProtein.exists());
  state->setSubInference(getSubInference(index), inputProtein, state->getSupervision());
}

void ProteinSequentialInference::finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
{
  if (state->getSubOutput().exists())
  {
    const ProteinPtr& workingProtein = state->getSubOutput().getObjectAndCast<Protein>();
    jassert(workingProtein);
    saveDebugFiles(workingProtein, state->getStepNumber());
  }
}

Variable ProteinSequentialInference::finalizeInference(InferenceContextPtr context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {return finalState->getSubOutput();} // latest version of the working protein

/*
** ProteinParallelInference
*/
ProteinParallelInference::ProteinParallelInference(const String& name)
  : VectorParallelInference(name)
{
}

ParallelInferenceStatePtr ProteinParallelInference::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  if (supervision.exists())
    prepareSupervisionProtein(supervision.getObjectAndCast<Protein>());

  ProteinPtr inputProtein = prepareInputProtein(input);
  ParallelInferenceStatePtr state(new ParallelInferenceState(inputProtein, supervision));
  size_t n = getNumSubInferences();
  state->reserve(n);
  jassert(n);
  for (size_t i = 0; i < n; ++i)
    state->addSubInference(getSubInference(i), inputProtein, supervision);
  return state;
}

Variable ProteinParallelInference::finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
{
  for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    jassert(state->getSubOutput(i) == state->getSubInput(i)); // ProteinParallelInference only accept sub-inference that do side-effects into the input protein
  saveDebugFiles(state->getInput().getObjectAndCast<Protein>(), 0);
  return state->getInput();
}


/*
** ProteinInferenceStep
*/
ProteinInferenceStep::ProteinInferenceStep(const String& targetName, InferencePtr targetInference)
  : StaticDecoratorInference(targetName, targetInference)
{
  int index = proteinClass->findObjectVariable(targetName);
  jassert(index >= 0);
  targetIndex = (size_t)index;
  checkInheritance((TypePtr)proteinClass, targetInference->getInputType());
  checkInheritance(getTargetType(), targetInference->getSupervisionType());
  // FIXME: we need the ability to declare variables with abstract types
  // here, getTargetType() is the concrete GenericVector<Probability> class, whil
  // the inference target is the generic Vector<Probability> class.
  // getTargetType() should have been typed with the generic type !
  //checkInheritance(targetInference->getOutputType(proteinClass), getTargetType());
}

DecoratorInferenceStatePtr ProteinInferenceStep::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
  ObjectPtr supervisionObject = supervision.getObject();
  res->setSubInference(decorated, input, supervisionObject ? supervisionObject->getVariable(targetIndex) : Variable());
  return res;
}

Variable ProteinInferenceStep::finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
{
  const ProteinPtr& protein = finalState->getInput().getObjectAndCast<Protein>();
  Variable prediction = finalState->getSubOutput();
  if (prediction.exists())
    protein->setVariable(targetIndex, prediction);
  return protein;
}
