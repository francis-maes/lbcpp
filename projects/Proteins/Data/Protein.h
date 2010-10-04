/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_H_
# define LBCPP_PROTEINS_PROTEIN_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Data/ProbabilityDistribution.h>

# include "AminoAcid.h"
# include "SecondaryStructure.h"
# include "TertiaryStructure.h"

namespace lbcpp
{

class Protein;
typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;
  
class Protein : public NameableObject
{
public:
  Protein(const String& name)
    : NameableObject(name) {}

  Protein() {}
  
  /*
  ** Save/Load operators
  */
  static ProteinPtr createFromPDB(const File& pdbFile, bool beTolerant = true, MessageCallback& callback = MessageCallback::getInstance());
  static ProteinPtr createFromXml(const File& file, MessageCallback& callback = MessageCallback::getInstance());
  static ProteinPtr createFromFASTA(const File& file, MessageCallback& callback = MessageCallback::getInstance());

  void saveToPDBFile(const File& pdbFile, MessageCallback& callback = MessageCallback::getInstance()) const;
  void saveToXmlFile(const File& xmlFile, MessageCallback& callback = MessageCallback::getInstance()) const;
  void saveToFASTAFile(const File& fastaFile, MessageCallback& callback = MessageCallback::getInstance()) const;

  Variable createEmptyTarget(size_t index) const;
  static String getTargetFriendlyName(size_t index);
  VectorPtr createEmptyProbabilitySequence() const;
  
  size_t getLength() const
    {return primaryStructure ? primaryStructure->getNumElements() : 0;}

  /*
  ** Primary Structure
  */
  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

  void setPrimaryStructure(VectorPtr primaryStructure)
    {this->primaryStructure = primaryStructure;}

  void setPrimaryStructure(const String& primaryStructure);
  
  VectorPtr getPositionSpecificScoringMatrix() const
    {return positionSpecificScoringMatrix;}

  void setPositionSpecificScoringMatrix(VectorPtr pssm)
    {positionSpecificScoringMatrix = pssm;}
  
  VectorPtr createEmptyPositionSpecificScoringMatrix() const;

  /*
  ** Secondary Structure
  */
  void setSecondaryStructure(VectorPtr secondaryStructure)
    {this->secondaryStructure = secondaryStructure;}

  VectorPtr getSecondaryStructure() const
    {return secondaryStructure;}

  VectorPtr createEmptySecondaryStructure() const;

  void setDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure)
    {this->dsspSecondaryStructure = dsspSecondaryStructure;}

  VectorPtr getDSSPSecondaryStructure() const
    {return dsspSecondaryStructure;}

  VectorPtr createEmptyDSSPSecondaryStructure() const;
  
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
  {jassert(disorderRegions->getElementsType() == probabilityType()); this->disorderRegions = disorderRegions;}

  VectorPtr getDisorderRegions() const
    {return disorderRegions;}
  
  /*
  ** Contact maps / Distance maps
  */
  SymmetricMatrixPtr getContactMap(double threshold = 8, bool betweenCBetaAtoms = false) const;
  void setContactMap(SymmetricMatrixPtr contactMap, double threshold = 8, bool betweenCBetaAtoms = false);
  SymmetricMatrixPtr createEmptyContactMap() const;

  SymmetricMatrixPtr getDistanceMap(bool betweenCBetaAtoms = false) const;
  void setDistanceMap(SymmetricMatrixPtr contactMap, bool betweenCBetaAtoms = false);
  SymmetricMatrixPtr createEmptyDistanceMap() const;

  /*
  ** Tertiary Structure
  */
  CartesianPositionVectorPtr getCAlphaTrace() const
    {return calphaTrace;}

  void setCAlphaTrace(CartesianPositionVectorPtr calphaTrace)
    {this->calphaTrace = calphaTrace;}

  TertiaryStructurePtr getTertiaryStructure() const
    {return tertiaryStructure;}

  void setTertiaryStructure(TertiaryStructurePtr tertiaryStructure)
    {this->tertiaryStructure = tertiaryStructure;}

  /*
  ** Compute Missing Variables
  */
  void computeMissingVariables();

  juce_UseDebuggingNewOperator

protected:
  friend class ProteinClass;

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
  TertiaryStructurePtr tertiaryStructure;

  static VectorPtr computeDisorderRegionsFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure);
  static VectorPtr computeSecondaryStructureFromDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure);
  static VectorPtr computeBinarySolventAccessibilityFromSolventAccessibility(VectorPtr solventAccessibility, double threshold);
  static SymmetricMatrixPtr computeContactMapFromDistanceMap(SymmetricMatrixPtr distanceMap, double threshold);
  static VectorPtr computeStructuralAlphabetSequenceFromCAlphaTrace(CartesianPositionVectorPtr calphaTrace);

  static CartesianPositionVectorPtr computeCAlphaTraceFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure)
    {return tertiaryStructure->makeCAlphaTrace();}

  static SymmetricMatrixPtr computeDistanceMapFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure, bool betweenCBetaAtoms)
    {return betweenCBetaAtoms ? tertiaryStructure->makeCBetaDistanceMatrix() : tertiaryStructure->makeCAlphaDistanceMatrix();}
};

extern ClassPtr proteinClass();

extern TypePtr angstromDistanceType();

extern FunctionPtr proteinLengthFunction();
extern FunctionPtr proteinToInputOutputPairFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
