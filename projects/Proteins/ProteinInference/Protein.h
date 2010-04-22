/*-----------------------------------------.---------------------------------.
| Filename: Protein.h                      | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_H_

# include "../InferenceData/LabelSequence.h"
# include "../InferenceData/ScoreVectorSequence.h"
# include "../InferenceData/CartesianCoordinatesSequence.h"
# include "../InferenceData/BondCoordinatesSequence.h"
# include "../InferenceData/ScoreSymmetricMatrix.h"
# include "ProteinTertiaryStructure.h"

namespace lbcpp
{

class Protein;
typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;

class Protein : public StringToObjectMap
{
public:
  Protein(const String& name)
    : StringToObjectMap(name) {}
  Protein() {}

  static ProteinPtr createFromAminoAcidSequence(const String& name, const String& aminoAcidSequence);
  static ProteinPtr createFromFASTA(const File& fastaFile);
  static ProteinPtr createFromPDB(const File& pdbFile, bool beTolerant = true);
  
  static ProteinPtr createFromFile(const File& proteinFile)
    {return Object::createFromFileAndCast<Protein>(proteinFile);}

  void saveToFASTAFile(const File& fastaFile);
  void saveToPDBFile(const File& pdbFile);

  /*
  ** Primary Structure, Position Specific Scoring Matrix and Properties
  */
  size_t getLength() const;
  LabelSequencePtr getAminoAcidSequence() const;
  ScoreVectorSequencePtr getPositionSpecificScoringMatrix() const;
  ScoreVectorSequencePtr getAminoAcidProperty() const;

  /*
  ** Secondary Structure
  */
  LabelSequencePtr getSecondaryStructureSequence() const;
  LabelSequencePtr getDSSPSecondaryStructureSequence() const;

  /*
  ** Solvent Accesibility
  */
  LabelSequencePtr getSolventAccessibilitySequence() const;

  /*
  ** Order/Disorder
  */
  LabelSequencePtr getOrderDisorderSequence() const;
  ScoreVectorSequencePtr getOrderDisorderScoreSequence() const;

  /*
  ** Residue-residue distance
  */
  ScoreSymmetricMatrixPtr getResidueResidueContactProbabilityMatrix() const; // contact at 8 angstrom

  /*
  ** Tertiary structure
  */
  ProteinDihedralAnglesPtr getDihedralAngles() const;
  CartesianCoordinatesSequencePtr getCAlphaTrace() const;
  ProteinTertiaryStructurePtr getTertiaryStructure() const;
  
  /*
  ** Compute some information
  */
  void computeProperties();

protected:
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
