/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PROTEIN_H_
# define LBCPP_PROTEINS_PROTEIN_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Distribution/Distribution.h>

# include "AminoAcid.h"
# include "SecondaryStructure.h"
# include "TertiaryStructure.h"

namespace lbcpp
{

enum ProteinTarget
{
  noTarget = 0, // corresponds to "name" variable
  aaTarget,
  pssmTarget,
  cbpTarget,
  ss3Target,
  ss8Target,
  stalTarget,
  saTarget,
  sa20Target,
  drTarget,
  cma8Target,
  cmb8Target,
  dmaTarget,
  dmbTarget,
  dsbTarget,
  cbsTarget

  // todo: continue
};

inline void variableToNative(ExecutionContext& context, ProteinTarget& dest, const Variable& source)
  {jassert(source.isEnumeration()); dest = (ProteinTarget)source.getInteger();}

inline void variableToNative(ExecutionContext& context, std::vector<ProteinTarget>& dest, const Variable& source)
{
  ContainerPtr sourceContainer = source.dynamicCast<Container>();
  jassert(sourceContainer);
  dest.resize(sourceContainer->getNumElements());
  for (size_t i = 0; i < dest.size(); ++i)
    dest[i] = (ProteinTarget)sourceContainer->getElement(i).getInteger();
}

template<class TT>
inline void variableToNative(ExecutionContext& context, std::pair<ProteinTarget, TT>& dest, const Variable& source)
{
  jassert(source.isObject());
  const PairPtr& sourcePair = source.getObjectAndCast<Pair>(context);
  if (sourcePair)
  {
    lbcpp::variableToNative(context, dest.first, sourcePair->getFirst());
    lbcpp::variableToNative(context, dest.second, sourcePair->getSecond());
  }
  else
  {
    dest.first = noTarget;
    dest.second = TT();
  }
}

template<class TT> // FIXME: is it still necessary ? (julien)
inline void variableToNative(ExecutionContext& context, std::vector< std::pair<ProteinTarget, TT> >& dest, const Variable& source)
{
  jassert(source.isObject());
  const VectorPtr& sourceVector = source.getObjectAndCast<Vector>(context);
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
      lbcpp::variableToNative(context, dest[i], sourceVector->getElement(i));
  }
  else
    dest.clear();
}

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
  static ProteinPtr createFromPDB(ExecutionContext& context, const String pdbString, bool beTolerant = true);
  static ProteinPtr createFromXml(ExecutionContext& context, const File& file);
  static ProteinPtr createFromFASTA(ExecutionContext& context, const File& file);

  void saveToPDBFile(ExecutionContext& context, const File& pdbFile) const;
  void saveToXmlFile(ExecutionContext& context, const File& xmlFile) const;
  void saveToFASTAFile(ExecutionContext& context, const File& fastaFile) const;

  static ContainerPtr loadProteinsFromDirectory(ExecutionContext& context, const File& directory, size_t maxCount = 0, const String& workUnitName = T("Loading proteins"));
  static ContainerPtr loadProteinsFromDirectoryPair(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, size_t maxCount = 0, const String& workUnitName = T("Loading proteins"));

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  Variable getTargetOrComputeIfMissing(ExecutionContext& context, size_t targetIndex) const;

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

  ContainerPtr getPositionSpecificScoringMatrix() const
    {return positionSpecificScoringMatrix;}

  void setPositionSpecificScoringMatrix(ContainerPtr pssm)
    {positionSpecificScoringMatrix = pssm;}

  static ContainerPtr createEmptyPositionSpecificScoringMatrix(size_t length);

  /*
  ** Cystein Bonding Property
  */
  Variable getCysteinBondingProperty(ExecutionContext& context) const;

  void setCysteinBondingProperty(DoubleVectorPtr cysteinBondingProperty)
    {this->cysteinBondingProperty = cysteinBondingProperty;}

  /*
  ** Secondary Structure
  */
  void setSecondaryStructure(ContainerPtr secondaryStructure)
    {this->secondaryStructure = secondaryStructure;}

  ContainerPtr getSecondaryStructure() const;

  void setDSSPSecondaryStructure(ContainerPtr dsspSecondaryStructure)
    {this->dsspSecondaryStructure = dsspSecondaryStructure;}

  ContainerPtr getDSSPSecondaryStructure() const
    {return dsspSecondaryStructure;}

  static ContainerPtr createEmptyDSSPSecondaryStructure(size_t length, bool useSparseVectors);

  void setStructuralAlphabetSequence(VectorPtr structuralAlphabetSequence)
    {this->structuralAlphabetSequence = structuralAlphabetSequence;}

  ContainerPtr getStructuralAlphabetSequence() const;

  /*
  ** Solvent Accesibility
  */
  void setSolventAccessibility(VectorPtr solventAccessibility)
    {this->solventAccessibility = solventAccessibility;}

  DoubleVectorPtr getSolventAccessibility() const
    {return solventAccessibility;}

  void setSolventAccessibilityAt20p(DoubleVectorPtr solventAccessibilityAt20p)
    {this->solventAccessibilityAt20p = solventAccessibilityAt20p;}

  DoubleVectorPtr getSolventAccessibilityAt20p() const;

  static DoubleVectorPtr createEmptyProbabilitySequence(size_t length);

  static DoubleVectorPtr createEmptyDoubleSequence(size_t length);
  
  /*
  ** Disorder regions
  */
  void setDisorderRegions(DoubleVectorPtr disorderRegions)
    {jassert(disorderRegions->getElementsType() == probabilityType); this->disorderRegions = disorderRegions;}

  DoubleVectorPtr getDisorderRegions() const;
  
  /*
  ** Contact maps / Distance maps
  */
  SymmetricMatrixPtr getContactMap(ExecutionContext& context, double threshold = 8, bool betweenCBetaAtoms = false) const;
  void setContactMap(SymmetricMatrixPtr contactMap, double threshold = 8, bool betweenCBetaAtoms = false);
  static SymmetricMatrixPtr createEmptyContactMap(size_t length);

  SymmetricMatrixPtr getDistanceMap(ExecutionContext& context, bool betweenCBetaAtoms = false) const;
  void setDistanceMap(SymmetricMatrixPtr contactMap, bool betweenCBetaAtoms = false);
  static SymmetricMatrixPtr createEmptyDistanceMap(size_t length);

  /*
  ** Disulfide Bonds
  */
  const SymmetricMatrixPtr& getDisulfideBonds(ExecutionContext& context) const;
  void setDisulfideBonds(const SymmetricMatrixPtr& disulfideBonds)
    {jassert(disulfideBonds->getDimension() == cysteinIndices.size()); this->disulfideBonds = disulfideBonds;}

  const DoubleVectorPtr& getCysteinBondingStates(ExecutionContext& context) const;
  
  size_t getNumBondedCysteins() const;
  
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
  ContainerPtr positionSpecificScoringMatrix;
  std::vector<size_t> cysteinIndices;
  std::vector<int> cysteinInvIndices; // residue position => cystein index

  // 0D
  DoubleVectorPtr cysteinBondingProperty;

  // 1D
  ContainerPtr secondaryStructure;
  ContainerPtr dsspSecondaryStructure;
  ContainerPtr structuralAlphabetSequence;

  DoubleVectorPtr solventAccessibility;
  DoubleVectorPtr solventAccessibilityAt20p;

  DoubleVectorPtr disorderRegions;

  // 2D
  SymmetricMatrixPtr contactMap8Ca;
  SymmetricMatrixPtr contactMap8Cb;
  SymmetricMatrixPtr distanceMapCa;
  SymmetricMatrixPtr distanceMapCb;

  SymmetricMatrixPtr disulfideBonds;
  DoubleVectorPtr cysteinBondingStates;

  // 3D
  CartesianPositionVectorPtr calphaTrace;
  TertiaryStructurePtr tertiaryStructure;

  static DoubleVectorPtr computeCysteinBondingProperty(DoubleVectorPtr cysteinBondingStates);
  static DoubleVectorPtr computeDisorderRegionsFromTertiaryStructure(TertiaryStructurePtr tertiaryStructure);
  static ContainerPtr computeSecondaryStructureFromDSSPSecondaryStructure(ContainerPtr dsspSecondaryStructure);
  static DoubleVectorPtr computeBinarySolventAccessibilityFromSolventAccessibility(DoubleVectorPtr solventAccessibility, double threshold);
  static SymmetricMatrixPtr computeContactMapFromDistanceMap(SymmetricMatrixPtr distanceMap, double threshold);
  static ContainerPtr computeStructuralAlphabetSequenceFromCAlphaTrace(CartesianPositionVectorPtr calphaTrace);
  static SymmetricMatrixPtr computeDisulfideBondsFromTertiaryStructure(SymmetricMatrixPtr distanceMap);
  
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
