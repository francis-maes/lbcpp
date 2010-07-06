/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_H_
# define LBCPP_PROTEINS_PROTEIN_H_

# include <lbcpp/Object/Vector.h>
# include <lbcpp/Object/SymmetricMatrix.h>
# include "Residue.h"

namespace lbcpp
{

class Protein : public NameableObject
{
public:
  Protein(const String& name)
    : NameableObject(name) {}

  Protein() {}

  virtual Variable getVariable(size_t index) const;

  size_t getLength() const
    {return primaryStructure ? primaryStructure->size() : 0;}

  /*
  ** Low level representations
  */
  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

  void setPrimaryStructure(VectorPtr primaryStructure)
    {this->primaryStructure = primaryStructure;}

  void setSecondaryStructure(VectorPtr secondaryStructure)
    {this->secondaryStructure = secondaryStructure;}

  void setDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure)
    {this->dsspSecondaryStructure = dsspSecondaryStructure;}

  /*
  ** High level representation: residues graph
  */
  VectorPtr getResidues() const
    {return residues;}

  ResiduePtr getResidue(size_t index) const
    {return residues ? residues->getVariable(index).getObjectAndCast<Residue>() : ResiduePtr();}

  /*
  ** Compute Missing Variables
  */
  void computeMissingVariables();

protected:
  VectorPtr createResidues() const;

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

  VectorPtr residues;
};

extern ClassPtr proteinClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
