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

ProteinPtr ProteinInferenceHelper::cloneInputProtein(ExecutionContext& context, const Variable& input)
{
  const ProteinPtr& inputProtein = input.getObjectAndCast<Protein>(context);
  jassert(inputProtein);
  ProteinPtr workingProtein = inputProtein->cloneAndCast<Protein>(context);
  jassert(workingProtein);
  return workingProtein;
}

void ProteinInferenceHelper::prepareSupervisionProtein(ExecutionContext& context, ProteinPtr protein)
{
  jassert(protein);
  if (pdbDebugDirectory.exists() && protein->getTertiaryStructure())
    protein->saveToPDBFile(context, pdbDebugDirectory.getChildFile(protein->getName() + T("_correct.pdb")));
  if (proteinDebugDirectory.exists())
    protein->saveToXmlFile(context, proteinDebugDirectory.getChildFile(protein->getName() + T("_correct.xml")));
}

void ProteinInferenceHelper::saveDebugFiles(ExecutionContext& context, ProteinPtr protein, size_t stepNumber)
{
  String idx((int)stepNumber);

   if (pdbDebugDirectory.exists() &&  protein->getTertiaryStructure())
    protein->saveToPDBFile(context, pdbDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".pdb")));

  if (proteinDebugDirectory.exists())
    protein->saveToXmlFile(context, proteinDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".xml")));
}

/*
** ProteinSequentialInference
*/
ProteinSequentialInference::ProteinSequentialInference(const String& name)
  : VectorSequentialInference(name)
{
}

SequentialInferenceStatePtr ProteinSequentialInference::prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  if (supervision.exists())
    prepareSupervisionProtein(context, supervision.getObjectAndCast<Protein>(context));
  return VectorSequentialInference::prepareInference(context, cloneInputProtein(context, input), supervision, returnCode);
}

void ProteinSequentialInference::prepareSubInference(InferenceContext& context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
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

void ProteinSequentialInference::finalizeSubInference(InferenceContext& context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
{
  if (state->getSubOutput().exists())
  {
    const ProteinPtr& workingProtein = state->getSubOutput().getObjectAndCast<Protein>(context);
    jassert(workingProtein);
    saveDebugFiles(context, workingProtein, state->getStepNumber());
  }
}

Variable ProteinSequentialInference::finalizeInference(InferenceContext& context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {return finalState->getSubOutput();} // latest version of the working protein

/*
** ProteinParallelInference
*/
ProteinParallelInference::ProteinParallelInference(const String& name)
  : VectorParallelInference(name)
{
}

ParallelInferenceStatePtr ProteinParallelInference::prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  if (supervision.exists())
    prepareSupervisionProtein(context, supervision.getObjectAndCast<Protein>(context));

  ParallelInferenceStatePtr state(new ParallelInferenceState(input, supervision));
  size_t n = getNumSubInferences();
  state->reserve(n);
  jassert(n);
  for (size_t i = 0; i < n; ++i)
    state->addSubInference(getSubInference(i), input.clone(context), supervision);
  return state;
}

Variable ProteinParallelInference::finalizeInference(InferenceContext& context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
{
  const ProteinPtr& initialProtein = state->getInput().getObjectAndCast<Protein>(context);
  ProteinPtr finalProtein = initialProtein->cloneAndCast<Protein>(context);
  size_t numVariables = proteinClass->getObjectNumVariables();
  std::set<size_t> alreadySet;
  for (size_t i = 0; i < state->getNumSubInferences(); ++i)
  {
    ProteinPtr predictedProtein = state->getSubOutput(i).getObjectAndCast<Protein>(context);
    for (size_t j = 0; j < numVariables; ++j)
    {
      Variable initial = initialProtein->getVariable(j);
      Variable predicted = predictedProtein->getVariable(j);
      if (predicted.exists() && predicted != initial)
      {
        if (alreadySet.find(j) == alreadySet.end())
          alreadySet.insert(j);
        else
          context.warningCallback(T("ProteinParallelInference::finalizeInference"),
            T("More than one version of Protein::") + proteinClass->getObjectVariableName(j));
        finalProtein->setVariable(context, j, predicted);
      }
    }
  }
  saveDebugFiles(context, finalProtein, 0);
  return finalProtein;
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

DecoratorInferenceStatePtr ProteinInferenceStep::prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
  Variable targetSupervision;
  const ProteinPtr& supervisionProtein = supervision.getObjectAndCast<Protein>(context);
  if (supervisionProtein)
    targetSupervision = supervisionProtein->getTargetOrComputeIfMissing(targetIndex);
  if (targetSupervision.isNil())
    targetSupervision = Variable::missingValue(proteinClass->getObjectVariableType(targetIndex));
  res->setSubInference(decorated, input, targetSupervision);
  return res;
}

Variable ProteinInferenceStep::finalizeInference(InferenceContext& context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
{
  const ProteinPtr& protein = finalState->getInput().getObjectAndCast<Protein>(context);
  Variable prediction = finalState->getSubOutput();
  if (prediction.exists())
    protein->setVariable(context, targetIndex, prediction);
  return protein;
}
