/*-----------------------------------------.---------------------------------.
| Filename: ProteinObject.h                | ProteinObject                   |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_H_

# include "../InferenceData/LabelSequence.h"
# include "../InferenceData/ScalarSequence.h"
# include "../InferenceData/ScoreVectorSequence.h"
# include "../InferenceData/CartesianCoordinatesSequence.h"
# include "../InferenceData/BondCoordinatesSequence.h"
# include "../InferenceData/ScoreSymmetricMatrix.h"
//# include "Data/AminoAcid.h"
//# include "Data/SecondaryStructure.h"
# include "ProteinTertiaryStructure.h"

namespace lbcpp
{

class ProteinObject;
typedef ReferenceCountedObjectPtr<ProteinObject> ProteinObjectPtr;

class ProteinObject : public StringToObjectMap
{
public:
  ProteinObject(const String& name)
    : StringToObjectMap(name), versionNumber(0) {}
  ProteinObject() : versionNumber(0) {}

  static ProteinObjectPtr createFromAminoAcidSequence(const String& name, const String& aminoAcidSequence);
  static ProteinObjectPtr createFromFASTA(const File& fastaFile);
  static ProteinObjectPtr createFromPDB(const File& pdbFile, bool beTolerant = true);
  
  static ProteinObjectPtr createFromFile(const File& proteinFile)
    {return Object::createFromFileAndCast<ProteinObject>(proteinFile);}

  void saveToFASTAFile(const File& fastaFile);
  void saveToPDBFile(const File& pdbFile);

  void computeMissingFields();

  ObjectPtr createEmptyObject(const String& name) const;

  /*
  ** Primary Structure, Position Specific Scoring Matrix and Properties
  */
  size_t getLength() const;
  LabelSequencePtr getAminoAcidSequence() const;
  ScoreVectorSequencePtr getPositionSpecificScoringMatrix() const;

  ScoreVectorSequencePtr getAminoAcidProperty() const;
  LabelSequencePtr getReducedAminoAcidAlphabetSequence() const;
  LabelSequencePtr getStructuralAlphabetSequence() const;

  /*
  ** Secondary Structure
  */
  LabelSequencePtr getSecondaryStructureSequence() const;
  ScoreVectorSequencePtr getSecondaryStructureProbabilities() const;
  
  LabelSequencePtr getDSSPSecondaryStructureSequence() const;
  ScoreVectorSequencePtr getDSSPSecondaryStructureProbabilities() const;

  /*
  ** Solvent Accesibility
  */
  ScalarSequencePtr getNormalizedSolventAccessibilitySequence() const;
  LabelSequencePtr getSolventAccessibilityThreshold20() const;

  /*
  ** Order/Disorder
  */
  LabelSequencePtr getDisorderSequence() const;
  ScalarSequencePtr getDisorderProbabilitySequence() const;

  /*
  ** Residue-residue distance
  */
  ScoreSymmetricMatrixPtr getResidueResidueDistanceMatrixCa() const; // distance matrix between Ca atoms
  ScoreSymmetricMatrixPtr getResidueResidueContactMatrix8Ca() const; // contact at 8 angstrom between Ca atoms
  ScoreSymmetricMatrixPtr getResidueResidueDistanceMatrixCb() const; // distance matrix between Cb atoms
  ScoreSymmetricMatrixPtr getResidueResidueContactMatrix8Cb() const; // contact at 8 angstrom between Cb atoms


  /*
  ** Tertiary structure
  */
  CartesianCoordinatesSequencePtr getCAlphaTrace() const;
  BondCoordinatesSequencePtr getCAlphaBondSequence() const;
  ProteinBackboneBondSequencePtr getBackboneBondSequence() const;
  ProteinTertiaryStructurePtr getTertiaryStructure() const;
  
  /*
  ** List of existing sequences
  */
  void getLabelSequences(std::vector<LabelSequencePtr>& results);
  
  /*
  ** Compute some information
  */
  void computePropertiesFrom(const std::vector< ScalarSequencePtr >& aaindex);

  void setVersionNumber(size_t versionNumber)
    {this->versionNumber = versionNumber;}

  size_t getVersionNumber() const
    {return versionNumber;}

protected:
  size_t versionNumber;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
