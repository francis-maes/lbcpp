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
    {return proteinClass();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return sequenceSeparationDistanceType();}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    ProteinPtr protein = input.getObjectAndCast<Protein>();
    jassert(protein);
    return Variable(protein->getLength(), sequenceSeparationDistanceType());
  }
};

/*
** ProteinToVariableFunction
*/
class ProteinToVariableFunction : public Function
{
public:
  ProteinToVariableFunction(int index = -1) : index(index) {}
  
  virtual TypePtr getInputType() const
    {return proteinClass();}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType->getObjectVariableType((size_t)index);}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(index >= 0 && index < (int)input.getObject()->getNumVariables());
    VectorPtr vector = input.getObject()->getVariable(index).getObjectAndCast<Vector>();

    if (!vector)
      return Variable::missingValue(proteinClass()->getObjectVariableType((size_t)index));
    return vector;
  }

private:
  friend class ProteinToVariableFunctionClass;
  
  int index;
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

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
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
