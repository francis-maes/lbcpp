/*-----------------------------------------.---------------------------------.
| Filename: ResiduePerception.h            | Residue Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_RESIDUE_H_

# include <lbcpp/Data/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ResidueCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern PerceptionPtr proteinToResiduePerception(PerceptionPtr proteinPerception);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_RESIDUE_H_
