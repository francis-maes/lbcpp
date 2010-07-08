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
# include <lbcpp/Object/ProbabilityDistribution.h>

# include "AminoAcid.h"
# include "SecondaryStructure.h"

namespace lbcpp
{

class Protein : public NameableObject
{
public:
  Protein(const String& name)
    : NameableObject(name) {}

  Protein() {}

  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);

  size_t getLength() const
    {return primaryStructure ? primaryStructure->size() : 0;}

  /*
  ** Primary Structure
  */
  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

  void setPrimaryStructure(VectorPtr primaryStructure)
    {this->primaryStructure = primaryStructure;}

  VectorPtr getPositionSpecificScoringMatrix() const
    {return positionSpecificScoringMatrix;}

  void setPositionSpecificScoringMatrix(VectorPtr pssm)
    {positionSpecificScoringMatrix = pssm;}

  /*
  ** Secondary Structure
  */
  void setSecondaryStructure(VectorPtr secondaryStructure)
    {this->secondaryStructure = secondaryStructure;}

  VectorPtr getSecondaryStructure() const
    {return secondaryStructure;}

  void setDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure)
    {this->dsspSecondaryStructure = dsspSecondaryStructure;}

  VectorPtr getDSSPSecondaryStructure() const
    {return dsspSecondaryStructure;}

  /*
  ** Compute Missing Variables
  */
  void computeMissingVariables();

protected:
  VectorPtr primaryStructure;
  VectorPtr positionSpecificScoringMatrix;

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

typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;
extern ClassPtr proteinClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
