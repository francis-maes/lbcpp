/*-----------------------------------------.---------------------------------.
| Filename: SeparationDistanceResiduePair.h| Separation Distance Perception  |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_SEPARATION_DISTANCE_RESIDUE_PAIR_H_
# define LBCPP_PROTEIN_PERCEPTION_SEPARATION_DISTANCE_RESIDUE_PAIR_H_

# include "ProteinPerception.h"

namespace lbcpp
{

class SeparationDistanceResiduePairPerception : public Perception
{
public:
  SeparationDistanceResiduePairPerception()
    {computeOutputType();}

  virtual TypePtr getInputType() const
    {return pairClass(proteinClass, pairClass(positiveIntegerType, positiveIntegerType));}

  virtual void computeOutputType()
  {
    addOutputVariable(T("separationDistance"), positiveIntegerType);
    Perception::computeOutputType();
  }
  
  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const PairPtr& positionPair = pair->getSecond().getObjectAndCast<Pair>();
    callback->sense(0, abs(positionPair->getSecond().getInteger() - positionPair->getSecond().getInteger()));
  }

protected:
  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_SEPARATION_DISTANCE_RESIDUE_PAIR_H_
