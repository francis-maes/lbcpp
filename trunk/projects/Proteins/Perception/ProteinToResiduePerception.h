/*-----------------------------------------.---------------------------------.
| Filename: ProteinToResiduePerception.h   | Protein -> Residue Perception   |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_H_

# include <lbcpp/Data/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

/*
** ProteinToResiduePerception
*/
class ProteinToResiduePerception : public DecoratorPerception
{
public:
  ProteinToResiduePerception(PerceptionPtr proteinPerception = PerceptionPtr())
    : DecoratorPerception(proteinPerception) {}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {decorated->computePerception(input[0], callback);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_TO_RESIDUE_H_
