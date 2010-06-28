/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_H_
# define LBCPP_PROTEINS_PROTEIN_H_

# include "Residue.h"
# include <lbcpp/Object/Vector.h>
# include <lbcpp/Object/SymmetricMatrix.h>

namespace lbcpp
{

class Protein : public NameableObject
{
public:
  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

protected:
  VectorPtr primaryStructure;
  VectorPtr secondaryStructure;
  VectorPtr dsspSecondaryStructure;
  VectorPtr solventAccessibility;
  VectorPtr solventAccessibilityAt20p;
  VectorPtr disorderRegions;
  VectorPtr disorderRegionProbabilities;

  SymmetricMatrixPtr contactMap8Ca;
  SymmetricMatrixPtr contactMap8Cb;
  SymmetricMatrixPtr distanceMapCa;
  SymmetricMatrixPtr distanceMapCb;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
