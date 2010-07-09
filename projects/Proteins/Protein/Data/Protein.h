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
# include "CartesianPositionVector.h"

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

  void setStructuralAlphabetSequence(VectorPtr structuralAlphabetSequence)
    {this->structuralAlphabetSequence = structuralAlphabetSequence;}

  VectorPtr getStructuralAlphabetSequence() const
    {return structuralAlphabetSequence;}

  /*
  ** Solvent Accesibility
  */
  void setSolventAccessibility(VectorPtr solventAccessibility)
    {this->solventAccessibility = solventAccessibility;}

  VectorPtr getSolventAccessibility() const
    {return solventAccessibility;}

  void setSolventAccessibilityAt20p(VectorPtr solventAccessibilityAt20p)
    {this->solventAccessibilityAt20p = solventAccessibilityAt20p;}

  VectorPtr getSolventAccessibilityAt20p() const
    {return solventAccessibilityAt20p;}

  /*
  ** Disorder regions
  */
  void setDisorderRegions(VectorPtr disorderRegions)
    {this->disorderRegions = disorderRegions;}

  VectorPtr getDisorderRegions() const
    {return disorderRegions;}

  /*
  ** Contact maps
  */
  SymmetricMatrixPtr getContactMap(double threshold = 8, bool betweenCBetaAtoms = false) const;
  void setContactMap(SymmetricMatrixPtr contactMap, double threshold = 8, bool betweenCBetaAtoms = false);

  /*
  ** Tertiary Structure
  */
  CartesianPositionVectorPtr getCAlphaTrace() const
    {return calphaTrace;}

  void setCAlphaTrace(CartesianPositionVectorPtr calphaTrace)
    {this->calphaTrace = calphaTrace;}

  /*
  ** Compute Missing Variables
  */
  void computeMissingVariables();

protected:
  // input
  VectorPtr primaryStructure;
  VectorPtr positionSpecificScoringMatrix;

  // 1D
  VectorPtr secondaryStructure;
  VectorPtr dsspSecondaryStructure;
  VectorPtr structuralAlphabetSequence;

  VectorPtr solventAccessibility;
  VectorPtr solventAccessibilityAt20p;

  VectorPtr disorderRegions;

  // 2D
  SymmetricMatrixPtr contactMap8Ca;
  SymmetricMatrixPtr contactMap8Cb;
  SymmetricMatrixPtr distanceMapCa;
  SymmetricMatrixPtr distanceMapCb;

  // 3D
  CartesianPositionVectorPtr calphaTrace;
};

typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;
extern ClassPtr proteinClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
