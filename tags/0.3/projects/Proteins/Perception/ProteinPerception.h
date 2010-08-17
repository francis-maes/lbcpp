/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

# include <lbcpp/Data/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ProteinCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}
};

PerceptionPtr proteinLengthPerception();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
