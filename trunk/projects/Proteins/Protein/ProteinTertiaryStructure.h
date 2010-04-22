/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.h     | Protein Tertiary Structure      |
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
# include "AminoAcidDictionary.h"
# include "ProteinResidue.h"

namespace lbcpp
{

// predeclarations
class ProteinDihedralAngles;
typedef ReferenceCountedObjectPtr<ProteinDihedralAngles> ProteinDihedralAnglesPtr;

class ProteinTertiaryStructure;
typedef ReferenceCountedObjectPtr<ProteinTertiaryStructure> ProteinTertiaryStructurePtr;

class ProteinDihedralAngles : public BuiltinVectorBasedSequence<DihedralAnglesPair>
{
public:
  typedef BuiltinVectorBasedSequence<DihedralAnglesPair> BaseClass;

  ProteinDihedralAngles(size_t length) : BaseClass(T("DihedralAngles"), length) {}
  ProteinDihedralAngles() {}
 
  static ProteinDihedralAnglesPtr createDihedralAngles(ProteinTertiaryStructurePtr tertiaryStructure);

  DihedralAnglesPair getAnglesPair(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  DihedralAngle getPhi(size_t index) const
    {return getAnglesPair(index).first;}

  DihedralAngle getPsi(size_t index) const
    {return getAnglesPair(index).second;}

  void setAnglesPair(size_t index, DihedralAngle phi, DihedralAngle psi)
    {jassert(index < elements.size()); elements[index] = DihedralAnglesPair(phi, psi);}
};

class ProteinTertiaryStructure : public Sequence
{
public:
  ProteinTertiaryStructure(size_t numResidues);
  ProteinTertiaryStructure() {}

  static ProteinTertiaryStructurePtr createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, CartesianCoordinatesSequencePtr trace);
  static ProteinTertiaryStructurePtr createFromDihedralAngles(LabelSequencePtr aminoAcidSequence, ProteinDihedralAnglesPtr dihedralAngles);

  LabelSequencePtr createAminoAcidSequence() const;
  CartesianCoordinatesSequencePtr createCAlphaTrace() const;
  CartesianCoordinatesSequencePtr createCBetaTrace() const;

  virtual size_t size() const
    {return residues.size();}

  virtual ObjectPtr get(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  virtual void resize(size_t newSize)
    {residues.resize(newSize);}

  virtual void set(size_t index, ObjectPtr object);

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return FeatureGeneratorPtr();} // todo
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();} // todo

  ProteinResiduePtr getResidue(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  ProteinResiduePtr getLastResidue() const
    {return residues.size() ? residues.back() : ProteinResiduePtr();}

  void setResidue(size_t index, ProteinResiduePtr residue)
    {jassert(index < residues.size()); residues[index] = residue;}

  void append(ProteinResiduePtr residue)
    {residues.push_back(residue);}

  bool hasOnlyCAlphaAtoms() const;
  bool isConsistent(String& failureReason) const;

  void pruneResiduesThatDoNotHaveCompleteBackbone();
  size_t getNumSpecifiedResidues() const;

private:
  std::vector<ProteinResiduePtr> residues;

  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, residues);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, residues);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
