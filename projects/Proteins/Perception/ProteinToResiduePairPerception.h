/*-----------------------------------------.---------------------------------.
| Filename: ProteinToResiduePairPerception.h| Protein -> Residue Pair        |
| Author  : Francis Maes                   |  Perception                     |
| Started : 19/08/2010 11:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_PAIR_H_
# define LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_PAIR_H_

# include "ProteinPerception.h"

namespace lbcpp
{

/*
** ProteinToResiduePairPerception
*/
class ProteinToResiduePairPerception : public DecoratorPerception
{
public:
  ProteinToResiduePairPerception(PerceptionPtr proteinPerception = PerceptionPtr())
    : DecoratorPerception(proteinPerception) {}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {decorated->computePerception(input[0], callback);}
};


/*
** ResidueToResiduePairPerception
*/
class ResidueToResiduePairPerception : public Perception
{
public:
  ResidueToResiduePairPerception(PerceptionPtr residuePerception = PerceptionPtr())
    : residuePerception(residuePerception) {}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(positiveIntegerType(), positiveIntegerType()));}

  virtual size_t getNumOutputVariables() const
    {return 2;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return residuePerception->getOutputType();}

  virtual String getOutputVariableName(size_t index) const
    {return index ? T("residue2") : T("residue1");}

  virtual PerceptionPtr getOutputVariableSubPerception(size_t index) const
    {return residuePerception;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    Variable protein = input[0];
    Variable positionPair = input[1];
    callback->sense(0, residuePerception, Variable::pair(protein, positionPair[0]));
    callback->sense(1, residuePerception, Variable::pair(protein, positionPair[1]));
  }

protected:
  friend class ResidueToResiduePairPerceptionClass;

  PerceptionPtr residuePerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_PAIR_H_
