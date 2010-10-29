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
  ProteinToInputOutputPairFunction()
    : outputType(pairClass(proteinClass, proteinClass)) {}

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return outputType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    ProteinPtr inputProtein = new Protein(protein->getName());
    inputProtein->setPrimaryStructure(protein->getPrimaryStructure());
    inputProtein->setPositionSpecificScoringMatrix(protein->getPositionSpecificScoringMatrix());
    return Variable::pair(inputProtein, protein, outputType);
  }

protected:
  TypePtr outputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DATA_FUNCTIONS_H_
