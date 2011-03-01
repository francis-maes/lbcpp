/*-----------------------------------------.---------------------------------.
| Filename: ProteinFunctions.h             | Protein Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DATA_FUNCTIONS_H_
# define LBCPP_PROTEIN_DATA_FUNCTIONS_H_

# include <lbcpp/Core/Function.h>
# include "Protein.h"

namespace lbcpp
{

/*
** ProteinLengthFunction
*/
class ProteinLengthFunction : public SimpleUnaryFunction
{
public:
  ProteinLengthFunction() : SimpleUnaryFunction(proteinClass, positiveIntegerType, T("Length")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    if (protein)
      return Variable(protein->getLength(), positiveIntegerType);
    else
      return Variable::missingValue(positiveIntegerType);
  }
};

/*
** ProteinToInputOutputPairFunction
*/
class ProteinToInputOutputPairFunction : public SimpleUnaryFunction
{
public:
  ProteinToInputOutputPairFunction(bool keepTertiaryStructure = true)
    : SimpleUnaryFunction(proteinClass, pairClass(proteinClass, proteinClass), T("IO")), keepTertiaryStructure(keepTertiaryStructure) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    if (!keepTertiaryStructure)
    {
      protein->getDisorderRegions(); // be sure that disordered regions are computed
      protein->getStructuralAlphabetSequence(); // be sure that structural alphabet is computed
      protein->getDisulfideBonds(context);

      protein->setTertiaryStructure(TertiaryStructurePtr()); // remove tertiary structure
      protein->setCAlphaTrace(CartesianPositionVectorPtr()); // remove c-alpha trace
      protein->setDistanceMap(SymmetricMatrixPtr(), false);
      protein->setDistanceMap(SymmetricMatrixPtr(), true);
    }

    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    //inputProtein->setSecondaryStructure(protein->getSecondaryStructure()); // FIXME: REMOVE
    return Variable::pair(inputProtein, protein, outputType);
  }

protected:
  friend class ProteinToInputOutputPairFunctionClass;

  bool keepTertiaryStructure;
};

// protein, (targetIndex, targetValue)* -> protein
class MakeProteinFunction : public Function
{
public:
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("protein");
    outputShortName = T("prot");
    return proteinClass;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinPtr inputProtein = inputs[0].getObjectAndCast<Protein>();
    if (!inputProtein)
      return Variable::missingValue(proteinClass);

    ProteinPtr res = inputProtein->cloneAndCast<Protein>();
    size_t numInputs = getNumInputs();
    for (size_t i = 1; i < numInputs; i += 2)
    {
      size_t targetIndex = (size_t)inputs[i].getInteger();
      const Variable& target = inputs[i + 1];
      if (target.exists())
      {
        jassert(targetIndex < proteinClass->getNumMemberVariables());
        res->setVariable(targetIndex, target);
      }
    }
    return res;
  }
};

// protein -> variable
class GetProteinTargetFunction : public SimpleUnaryFunction
{
public:
  GetProteinTargetFunction(ProteinTarget target = noTarget)
    : SimpleUnaryFunction(proteinClass, proteinClass->getMemberVariableType(target), T("Target")), target(target) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.getObjectAndCast<Protein>()->getTargetOrComputeIfMissing(context, target);}

protected:
  friend class GetProteinTargetFunctionClass;

  ProteinTarget target;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
