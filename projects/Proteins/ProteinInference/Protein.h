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

# include "AminoAcidDictionary.h" // for ProteinTertiaryStructure

namespace lbcpp
{

struct Vector3
{
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vector3() : x(0.0), y(0.0), z(0.0) {}

  double x, y, z;

  String toString() const
    {return T("(") + lbcpp::toString(x) + T(", ") + lbcpp::toString(y) + T(", ") + lbcpp::toString(z) + T(")");}
};

class ProteinCAlphaTrace : public Sequence
{
public:
  ProteinCAlphaTrace(size_t length)
    : Sequence(T("ProteinCAlphaTrace")), positions(length) {}

  virtual size_t size() const
    {return positions.size();}

  virtual ObjectPtr get(size_t index) const
    {return elementFeatures(index);}

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return unitFeatureGenerator();}

  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return unitFeatureGenerator();}

private:
  std::vector<Vector3> positions;
};

typedef ReferenceCountedObjectPtr<ProteinCAlphaTrace> ProteinCAlphaTracePtr;

class ProteinAtom : public NameableObject
{
public:
  ProteinAtom(const String& name, const String& elementSymbol)
    : NameableObject(name), elementSymbol(elementSymbol), occupancy(0.0), temperatureFactor(0.0) {}
  ProteinAtom() {}

  virtual String toString() const
    {return getName() + T(" ") + position.toString() + T(" ")
      + lbcpp::toString(occupancy) + T(" ") + lbcpp::toString(temperatureFactor);}

  String getElementSymbol() const
    {return elementSymbol;}

  const Vector3& getPosition() const
    {return position;}

  void setPosition(const Vector3& position)
    {this->position = position;}

  double getX() const
    {return position.x;}

  double getY() const
    {return position.y;}

  double getZ() const
    {return position.z;}

  double getOccupancy() const
    {return occupancy;}

  double getTemperatureFactor() const
    {return temperatureFactor;}

protected:
  String elementSymbol;
  Vector3 position;
  double occupancy;
  double temperatureFactor;
};

typedef ReferenceCountedObjectPtr<ProteinAtom> ProteinAtomPtr;

class ProteinResidue : public NameableObject
{
public:
  ProteinResidue(AminoAcidDictionary::Type aminoAcid)
    : aminoAcid(aminoAcid) {}

  virtual String getName() const
    {return AminoAcidDictionary::getThreeLettersCode(aminoAcid);}

  AminoAcidDictionary::Type getAminoAcid() const
    {return aminoAcid;}

  size_t getNumAtoms() const
    {return atoms.size();}

  ProteinAtomPtr getAtom(size_t index) const
    {jassert(index < atoms.size()); return atoms[index];}

  void addAtom(ProteinAtomPtr atom)
    {atoms.push_back(atom);}

protected:
  AminoAcidDictionary::Type aminoAcid;
  std::vector<ProteinAtomPtr> atoms;
};

typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

class ProteinTertiaryStructure : public Sequence
{
public:
  ProteinTertiaryStructure(size_t numResidues)
    : Sequence(T("TertiaryStructure")), residues(numResidues) {}

  virtual size_t size() const
    {return residues.size();}

  virtual ObjectPtr get(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  virtual void resize(size_t newSize)
    {residues.resize(newSize);}

  virtual void set(size_t index, ObjectPtr object)
  {
    ProteinResiduePtr residue = object.dynamicCast<ProteinResidue>();
    jassert(residue);
    jassert(index < residues.size());
    residues[index] = residue;
  }

  virtual FeatureGeneratorPtr elementFeatures(size_t position) const
    {return FeatureGeneratorPtr();} // todo
  virtual FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const
    {return FeatureGeneratorPtr();} // todo

  ProteinResiduePtr getResidue(size_t index) const
    {jassert(index < residues.size()); return residues[index];}

  void setResidue(size_t index, ProteinResiduePtr residue)
    {jassert(index < residues.size()); residues[index] = residue;}

private:
  std::vector<ProteinResiduePtr> residues;
};

typedef ReferenceCountedObjectPtr<ProteinTertiaryStructure> ProteinTertiaryStructurePtr;

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
