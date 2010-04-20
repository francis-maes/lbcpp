/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.h     | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
# define LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_

# include "AminoAcidDictionary.h"
# include "../InferenceData/LabelSequence.h"

# include "../Geometry/DihedralAngle.h"
# include "../Geometry/Vector3.h"
# include "../Geometry/Matrix4.h"

namespace lbcpp
{

// predeclarations
class ProteinCarbonTrace;
typedef ReferenceCountedObjectPtr<ProteinCarbonTrace> ProteinCarbonTracePtr;

class ProteinDihedralAngles;
typedef ReferenceCountedObjectPtr<ProteinDihedralAngles> ProteinDihedralAnglesPtr;

class ProteinAtom;
typedef ReferenceCountedObjectPtr<ProteinAtom> ProteinAtomPtr;

class ProteinResidue;
typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

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

class ProteinCarbonTrace : public BuiltinVectorBasedSequence<Vector3>
{
public:
  typedef BuiltinVectorBasedSequence<Vector3> BaseClass;

  ProteinCarbonTrace(const String& name, size_t length)
    : BaseClass(name, length) {}
  ProteinCarbonTrace() {}

  static ProteinCarbonTracePtr createCAlphaTrace(ProteinTertiaryStructurePtr tertiaryStructure);
  static ProteinCarbonTracePtr createCBetaTrace(ProteinTertiaryStructurePtr tertiaryStructure);

  Vector3 getPosition(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void setPosition(size_t index, const Vector3& position)
    {jassert(index < elements.size()); elements[index] = position;}
};

class ProteinAtom : public NameableObject
{
public:
  ProteinAtom(const String& name, const String& elementSymbol, const Vector3& position = Vector3())
    : NameableObject(name), elementSymbol(elementSymbol), position(position), occupancy(0.0), temperatureFactor(0.0) {}
  ProteinAtom() {}

  virtual String toString() const;

  String getElementSymbol() const
    {return elementSymbol;}

  const Vector3& getPosition() const
    {return position;}

  void setPosition(const Vector3& position)
    {this->position = position;}

  double getX() const
    {return position.getX();}

  double getY() const
    {return position.getY();}

  double getZ() const
    {return position.getZ();}

  void setOccupancy(double occupancy)
    {this->occupancy = occupancy;}

  double getOccupancy() const
    {return occupancy;}

  void setTemperatureFactor(double temperatureFactor)
    {this->temperatureFactor = temperatureFactor;}

  double getTemperatureFactor() const
    {return temperatureFactor;}

protected:
  String elementSymbol;
  Vector3 position;
  double occupancy;
  double temperatureFactor;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class ProteinResidue : public Object
{
public:
  ProteinResidue(AminoAcidDictionary::Type aminoAcid)
    : aminoAcid(aminoAcid) {}
  ProteinResidue()
    : aminoAcid(AminoAcidDictionary::unknown) {}

  virtual String toString() const;

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

  // central atom (back bone, first atom of the side chain)
  ProteinAtomPtr getCAlphaAtom() const
    {return findAtomByName(T("CA"));}

  // second carbon of the side chain
  ProteinAtomPtr getCBetaAtom() const
    {return findAtomByName(T("CB"));}

  // back bone
  ProteinAtomPtr getNitrogenAtom() const
    {return findAtomByName(T("N"));}

  // back bone
  ProteinAtomPtr getCarbonAtom() const
    {return findAtomByName(T("C"));}

  ProteinAtomPtr findAtomByName(const String& name) const;

  double getDistanceBetweenAtoms(const String& name1, const String& name2) const;
  double getDistanceBetweenAtoms(const String& name1, ProteinResiduePtr residue2, const String& name2) const;

protected:
  AminoAcidDictionary::Type aminoAcid;
  std::vector<ProteinAtomPtr> atoms;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class ProteinTertiaryStructure : public Sequence
{
public:
  ProteinTertiaryStructure(size_t numResidues);
  ProteinTertiaryStructure() {}

  static ProteinTertiaryStructurePtr createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, ProteinCarbonTracePtr trace);
  static ProteinTertiaryStructurePtr createFromDihedralAngles(LabelSequencePtr aminoAcidSequence, ProteinDihedralAnglesPtr dihedralAngles);

  LabelSequencePtr createAminoAcidSequence() const;

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

private:
  std::vector<ProteinResiduePtr> residues;

  virtual bool load(InputStream& istr)
    {return Sequence::load(istr) && lbcpp::read(istr, residues);}

  virtual void save(OutputStream& ostr) const
    {Sequence::save(ostr); lbcpp::write(ostr, residues);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_PROTEIN_TERTIARY_STRUCTURE_H_
