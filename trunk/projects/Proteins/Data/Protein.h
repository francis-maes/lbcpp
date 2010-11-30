/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_H_
# define LBCPP_PROTEINS_PROTEIN_H_

# include <lbcpp/Core/Vector.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/ProbabilityDistribution/ProbabilityDistribution.h>

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
  static ProteinPtr createFromPDB(ExecutionContext& context, const File& pdbFile, bool beTolerant = true);
  static ProteinPtr createFromXml(ExecutionContext& context, const File& file);
  static ProteinPtr createFromFASTA(ExecutionContext& context, const File& file);

  void saveToPDBFile(ExecutionContext& context, const File& pdbFile) const;
  void saveToXmlFile(ExecutionContext& context, const File& xmlFile) const;
  void saveToFASTAFile(ExecutionContext& context, const File& fastaFile) const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  Variable createEmptyTarget(size_t index) const;
  Variable getTargetOrComputeIfMissing(size_t variableIndex) const;

  VectorPtr createEmptyProbabilitySequence() const;
  
  size_t getLength() const
    {return primaryStructure ? primaryStructure->getNumElements() : 0;}

  /*
  ** Primary Structure
  */
  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

  const std::vector<size_t>& getCysteinIndices() const
    {return cysteinIndices;}

  const std::vector<int>& getCysteinInvIndices() const // residue position => cystein index
    {return cysteinInvIndices;}

  void setPrimaryStructure(VectorPtr primaryStructure);
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

  VectorPtr getSecondaryStructure() const;
  VectorPtr createEmptySecondaryStructure() const;

  void setDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure)
    {this->dsspSecondaryStructure = dsspSecondaryStructure;}

  VectorPtr getDSSPSecondaryStructure() const
    {return dsspSecondaryStructure;}

  VectorPtr createEmptyDSSPSecondaryStructure() const;
  
  void setStructuralAlphabetSequence(VectorPtr structuralAlphabetSequence)
    {this->structuralAlphabetSequence = structuralAlphabetSequence;}

  VectorPtr getStructuralAlphabetSequence() const;

  /*
  ** Solvent Accesibility
  */
  void setSolventAccessibility(VectorPtr solventAccessibility)
    {this->solventAccessibility = solventAccessibility;}

  VectorPtr getSolventAccessibility() const
    {return solventAccessibility;}

  void setSolventAccessibilityAt20p(VectorPtr solventAccessibilityAt20p)
    {this->solventAccessibilityAt20p = solventAccessibilityAt20p;}

  VectorPtr getSolventAccessibilityAt20p() const;

  /*
  ** Disorder regions
  */
  void setDisorderRegions(VectorPtr disorderRegions)
  {jassert(disorderRegions->getElementsType() == probabilityType); this->disorderRegions = disorderRegions;}

  VectorPtr getDisorderRegions() const;
  
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
  ** Disulfide Bonds
  */
  const SymmetricMatrixPtr& getDisulfideBonds() const;

  /*
  ** Tertiary Structure
  */
  CartesianPositionVectorPtr getCAlphaTrace() const;

  void setCAlphaTrace(const CartesianPositionVectorPtr& calphaTrace)
    {this->calphaTrace = calphaTrace;}

  TertiaryStructurePtr getTertiaryStructure() const
    {return tertiaryStructure;}

  void setTertiaryStructure(const TertiaryStructurePtr& tertiaryStructure)
    {this->tertiaryStructure = tertiaryStructure;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ProteinClass;

  // input
  VectorPtr primaryStructure;
  VectorPtr positionSpecificScoringMatrix;
  std::vector<size_t> cysteinIndices;
  std::vector<int> cysteinInvIndices; // residue position => cystein index

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

  SymmetricMatrixPtr disulfideBonds;


  // 3D
  CartesianPositionVectorPtr calphaTrace;
  TertiaryStructurePtr tertiaryStructure;

  static VectorPtr computeDisorderRegionsFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure);
  static VectorPtr computeSecondaryStructureFromDSSPSecondaryStructure(VectorPtr dsspSecondaryStructure);
  static VectorPtr computeBinarySolventAccessibilityFromSolventAccessibility(VectorPtr solventAccessibility, double threshold);
  static SymmetricMatrixPtr computeContactMapFromDistanceMap(SymmetricMatrixPtr distanceMap, double threshold);
  static VectorPtr computeStructuralAlphabetSequenceFromCAlphaTrace(CartesianPositionVectorPtr calphaTrace);
  static SymmetricMatrixPtr computeDisulfideBondsFromCBetaDistanceMap(const std::vector<size_t>& cysteines, SymmetricMatrixPtr distanceMap);

  static CartesianPositionVectorPtr computeCAlphaTraceFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure)
    {return tertiaryStructure->makeCAlphaTrace();}

  static SymmetricMatrixPtr computeDistanceMapFromTertiaryStructure(ExecutionContext& context, TertiaryStructurePtr tertiaryStructure, bool betweenCBetaAtoms)
    {return betweenCBetaAtoms ? tertiaryStructure->makeCBetaDistanceMatrix(context) : tertiaryStructure->makeCAlphaDistanceMatrix();}
};

extern ClassPtr proteinClass;

extern TypePtr angstromDistanceType;

extern FunctionPtr proteinLengthFunction();
extern FunctionPtr proteinToInputOutputPairFunction(bool keepTertiaryStructure = true);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PROTEIN_H_
