/*-----------------------------------------.---------------------------------.
| Filename: PositionResiduePerception.h    | Position Residue Perception     |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 14:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_POSITION_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_POSITION_RESIDUE_H_

# include "ProteinPerception.h"

namespace lbcpp
{

class PositionResiduePerception : public ResiduePerception
{
public:
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return probabilityType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("position");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ProteinPtr protein = input[0].getObjectAndCast<Protein>();
    jassert(protein);
    callback->sense(0, Variable((double)input[1].getInteger() / protein->getLength(), probabilityType()));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_POSITION_RESIDUE_H_
