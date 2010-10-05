/*-----------------------------------------.---------------------------------.
| Filename: BoundsProximityResiduePerce...h| Bounds Proximity Residue        |
| Author  : Francis Maes                   |  Perception                     |
| Started : 05/10/2010 14:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_BOUNDS_PROXIMITY_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_BOUNDS_PROXIMITY_RESIDUE_H_

# include "ProteinPerception.h"

namespace lbcpp
{

class BoundsProximityResiduePerception : public ResiduePerception
{
public:
  BoundsProximityResiduePerception(size_t outOfBoundWindowSize = 10)
    : outOfBoundWindowSize(outOfBoundWindowSize) {}
  
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return probabilityType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("terminus");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ProteinPtr protein = input[0].getObjectAndCast<Protein>();
    jassert(protein);
    size_t n = protein->getLength();
    size_t index = input[1].getInteger();
    
    if (index < outOfBoundWindowSize)
      callback->sense(0, Variable(index * (0.5 / outOfBoundWindowSize), probabilityType()));
    else if (n - index - 1 < outOfBoundWindowSize)
      callback->sense(0, Variable(0.5 + (outOfBoundWindowSize - (n - index - 1)) * (0.5 / outOfBoundWindowSize), probabilityType()));
    else
      callback->sense(0, Variable(0.5, probabilityType()));
  }

protected:
  friend class BoundsProximityResiduePerceptionClass;

  size_t outOfBoundWindowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_BOUNDS_PROXIMITY_RESIDUE_H_
