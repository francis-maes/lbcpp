/*-----------------------------------------.---------------------------------.
| Filename: ProteinFunctions.h             | Protein Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DATA_FUNCTIONS_H_
# define LBCPP_PROTEIN_DATA_FUNCTIONS_H_

# include <lbcpp/Function/Function.h>
# include "Protein.h"

namespace lbcpp
{

/*
** ProteinLengthFunction
*/
class ProteinLengthFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return positiveIntegerType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    return Variable(protein->getLength(), positiveIntegerType);
  }
};

/*
** ProteinToInputOutputPairFunction
*/
class ProteinToInputOutputPairFunction : public Function
{
public:
  ProteinToInputOutputPairFunction(bool keepTertiaryStructure)
    : outputType(pairClass(proteinClass, proteinClass)), keepTertiaryStructure(keepTertiaryStructure) {}
  ProteinToInputOutputPairFunction() {}

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return outputType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
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
    return Variable::pair(inputProtein, protein, outputType);
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!Function::loadFromXml(importer))
      return false;
    outputType = pairClass(proteinClass, proteinClass); // precompute output type
    return true;
  }

protected:
  friend class ProteinToInputOutputPairFunctionClass;

  TypePtr outputType;
  bool keepTertiaryStructure;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
