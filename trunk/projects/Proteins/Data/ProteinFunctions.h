/*-----------------------------------------.---------------------------------.
| Filename: ProteinFunctions.h             | Protein Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 12:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DATA_FUNCTIONS_H_
# define LBCPP_PROTEIN_DATA_FUNCTIONS_H_

# include <lbcpp/Data/Function.h>
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
    {return proteinClass();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return sequenceSeparationDistanceType();}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }
};

/*
** ProteinToInputOutputPairFunction
*/
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

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
