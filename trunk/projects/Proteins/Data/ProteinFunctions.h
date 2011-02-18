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
      protein->getDisulfideBonds();

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

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
