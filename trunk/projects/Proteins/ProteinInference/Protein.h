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

  /*
  ** Primary Structure and Position Specific Scoring Matrix
  */
  size_t getLength() const;
  LabelSequencePtr getAminoAcidSequence() const;
  ScoreVectorSequencePtr getPositionSpecificScoringMatrix() const;

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
  ProteinCAlphaTracePtr getCAlphaTrace() const;
  ProteinTertiaryStructurePtr getTertiaryStructure() const;

protected:
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_H_
