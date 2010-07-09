/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.h     | ProteinObject Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_

# include "../InferenceData/LabelSequence.h"
# include "../InferenceData/CartesianCoordinatesSequence.h"
# include "../InferenceData/BondCoordinatesSequence.h"
# include "../InferenceData/ScoreSymmetricMatrix.h"
# include "AminoAcidDictionary.h"
# include "ProteinResidueAtoms.h"
# include "ProteinBackboneBondSequence.h"

namespace lbcpp
{

class ProteinTertiaryStructure;
typedef ReferenceCountedObjectPtr<ProteinTertiaryStructure> ProteinTertiaryStructurePtr;

class ProteinTertiaryStructure : public Sequence
{
public:
  ProteinTertiaryStructure(size_t numResidues);
  ProteinTertiaryStructure() {}

  static ProteinTertiaryStructurePtr createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, CartesianCoordinatesSequencePtr trace);
  static ProteinTertiaryStructurePtr createFromBackbone(LabelSequencePtr aminoAcidSequence, ProteinBackboneBondSequencePtr backbone);

  LabelSequencePtr makeAminoAcidSequence() const;
  CartesianCoordinatesSequencePtr makeCAlphaTrace() const;
  CartesianCoordinatesSequencePtr makeCBetaTrace() const;
  ProteinBackboneBondSequencePtr makeBackbone() const;
  ScoreSymmetricMatrixPtr makeCAlphaDistanceMatrix() const;
  ScoreSymmetricMatrixPtr makeCBetaDistanceMatrix() const;

  virtual size_t size() const
    {return residues.size();}

  virtual ObjectPtr get(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  virtual void resize(size_t newSize)
    {residues.resize(newSize);}

  virtual void set(size_t index, ObjectPtr object);

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const;
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();} // todo
  virtual FeatureGeneratorPtr entropyFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();} // todo

  ProteinResidueAtomsPtr getResidue(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  ProteinResidueAtomsPtr getLastResidue() const
    {return residues.size() ? residues.back() : ProteinResidueAtomsPtr();}

  void setResidue(size_t index, ProteinResidueAtomsPtr residue)
    {jassert(index < residues.size()); residues[index] = residue;}

  void append(ProteinResidueAtomsPtr residue)
    {residues.push_back(residue);}

  bool hasCompleteBackbone() const;
  bool hasBackboneAndCBetaAtoms() const;
  bool hasCAlphaAtoms() const;

  bool isConsistent(String& failureReason) const;

  void pruneResiduesThatDoNotHaveCompleteBackbone();
  size_t getNumSpecifiedResidues() const;

  impl::Matrix4 superposeCAlphaAtoms(ProteinTertiaryStructurePtr targetStructure) const;
  double computeCAlphaAtomsRMSE(ProteinTertiaryStructurePtr targetStructure) const;
  void applyAffineTransform(const impl::Matrix4& affineTransform) const;

private:
  std::vector<ProteinResidueAtomsPtr> residues;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, residues);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
