/*-----------------------------------------.---------------------------------.
| Filename: ResidueToResiduePairPerception.h| Residue -> Residue Pair        |
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

class ResidueToResiduePairPerception : public Perception
{
public:
  ResidueToResiduePairPerception(PerceptionPtr residuePerception = PerceptionPtr())
    : residuePerception(residuePerception) {}

  virtual TypePtr getInputType() const
    {return pairClass(proteinClass(), pairClass(positiveIntegerType(), positiveIntegerType()));}

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
