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
  virtual TypePtr getInputType() const
    {return pairClass(proteinClass(), pairClass(positiveIntegerType(), positiveIntegerType()));}

  virtual size_t getNumOutputVariables() const
    {return 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return positiveIntegerType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("separationDistance");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    Variable positionPair = input[1];
    callback->sense(0, Variable(abs(positionPair[1].getInteger() - positionPair[0].getInteger()), positiveIntegerType()));
  }

protected:
  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_SEPARATION_DISTANCE_RESIDUE_PAIR_H_
