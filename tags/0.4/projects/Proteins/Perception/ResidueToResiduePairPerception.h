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
    : residuePerception(residuePerception) {computeOutputType();}

  virtual TypePtr getInputType() const
    {return pairClass(proteinClass, pairClass(positiveIntegerType, positiveIntegerType));}
  
  virtual void computeOutputType()
  {
    addOutputVariable(T("residue1"), residuePerception);
    addOutputVariable(T("residue2"), residuePerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    Variable protein = pair->getFirst();
    const PairPtr& positionPair = pair->getSecond().getObjectAndCast<Pair>();
    callback->sense(0, residuePerception, new Pair(protein, positionPair->getFirst()));
    callback->sense(1, residuePerception, new Pair(protein, positionPair->getSecond()));
  }

protected:
  friend class ResidueToResiduePairPerceptionClass;

  PerceptionPtr residuePerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_PAIR_H_
