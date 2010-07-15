/*-----------------------------------------.---------------------------------.
| Filename: ResiduePairPerception.h        | Residue Pair Perception         |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 17:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_RESIDUE_PAIR_H_
# define LBCPP_PROTEIN_PERCEPTION_RESIDUE_PAIR_H_

# include <lbcpp/Data/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ResiduePairCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern PerceptionPtr proteinToResiduePairPerception(PerceptionPtr proteinPerception);
extern PerceptionPtr residueToResiduePairPerception(PerceptionPtr residuePerception);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_RESIDUE_PAIR_H_
