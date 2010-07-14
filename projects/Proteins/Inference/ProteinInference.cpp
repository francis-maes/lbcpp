/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.cpp           | Protein Top Level Inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 01:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinInference.h"
#include <lbcpp/Data/Function.h>
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
  ProteinPtr inputProtein = input.getObjectAndCast<Protein>();
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
    protein->saveToXmlFile(proteinDebugDirectory.getChildFile(protein->getName() + T("_correct.xprot")));
}

void ProteinInferenceHelper::saveDebugFiles(ProteinPtr protein, size_t stepNumber)
{
  String idx((int)stepNumber);

   if (pdbDebugDirectory.exists() &&  protein->getTertiaryStructure())
    protein->saveToPDBFile(pdbDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".pdb")));

  if (proteinDebugDirectory.exists())
    protein->saveToXmlFile(proteinDebugDirectory.getChildFile
      (protein->getName() + T("_pred") + idx + T(".protein")));
}

/*
** ProteinSequentialInference
*/
ProteinSequentialInference::ProteinSequentialInference() : VectorSequentialInference(T("Protein"))
  {setBatchLearner(sequentialInferenceLearner());}

SequentialInferenceStatePtr ProteinSequentialInference::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  if (supervision)
    prepareSupervisionProtein(supervision.getObjectAndCast<Protein>());
  return VectorSequentialInference::prepareInference(context, prepareInputProtein(input), supervision, returnCode);
}

void ProteinSequentialInference::prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
{
  // we keep the same input and supervision for sub-inferences
  state->setSubInference(subInferences.get(index), state->getInput(), state->getSupervision());
}

void ProteinSequentialInference::finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
{
  if (state->getSubOutput())
  {
    ProteinPtr workingProtein = state->getInput().getObjectAndCast<Protein>();
    jassert(workingProtein);
    saveDebugFiles(workingProtein, state->getStepNumber());
  }
}

Variable ProteinSequentialInference::finalizeInference(InferenceContextPtr context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {return finalState->getInput();} // the working protein


class ProteinToInputOutputPairFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return pairType(proteinClass(), proteinClass());}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    protein->computeMissingVariables();
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    return Variable::pair(inputProtein, protein);
  }
};

FunctionPtr lbcpp::proteinToInputOutputPairFunction()
  {return new ProteinToInputOutputPairFunction();}

void declareProteinInferenceClasses()
{
  LBCPP_DECLARE_CLASS(ProteinSequentialInference, VectorSequentialInference); 
}
