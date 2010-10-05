/*-----------------------------------------.---------------------------------.
| Filename: IndexResiduePerception.h       | Index Residue Perception        |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 14:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_INDEX_RESIDUE_H_
# define LBCPP_PROTEIN_PERCEPTION_INDEX_RESIDUE_H_

# include "ProteinPerception.h"

namespace lbcpp
{

class IndexResiduePerception : public ResiduePerception
{
public:
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return positiveIntegerType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("index");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, input[1]);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_INDEX_RESIDUE_H_
