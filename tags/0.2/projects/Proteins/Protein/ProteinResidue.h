/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidue.h               | Protein Residue                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_RESIDUE_H_
# define LBCPP_PROTEIN_INFERENCE_RESIDUE_H_

# include "AminoAcidDictionary.h"
# include "../Geometry/Vector3.h"
# include "../Geometry/Matrix4.h"

namespace lbcpp
{

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
  
  Vector3& getPosition()
    {return position;}

  void setPosition(const Vector3& position)
    {this->position = position;}

  double getX() const
    {return position.getX();}

  void setX(double value)
    {position.setX(value);}

  double getY() const
    {return position.getY();}

  void setY(double value)
    {position.setY(value);}

  double getZ() const
    {return position.getZ();}

  void setZ(double value)
    {position.setZ(value);}

  void setOccupancy(double occupancy)
    {this->occupancy = occupancy;}

  double getOccupancy() const
    {return occupancy;}

  void setTemperatureFactor(double temperatureFactor)
    {this->temperatureFactor = temperatureFactor;}

  double getTemperatureFactor() const
    {return temperatureFactor;}

  FeatureGeneratorPtr positionFeatures() const;

protected:
  String elementSymbol;
  Vector3 position;
  double occupancy;
  double temperatureFactor;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

typedef ReferenceCountedObjectPtr<ProteinAtom> ProteinAtomPtr;

class ProteinResidue;
typedef ReferenceCountedObjectPtr<ProteinResidue> ProteinResiduePtr;

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

  void setAminoAcid(AminoAcidDictionary::Type aa)
    {aminoAcid = aa;}

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
  
  Vector3 getAtomPosition(const String& name) const
    {ProteinAtomPtr atom = findAtomByName(name); return atom ? atom->getPosition() : Vector3();}

  double getDistanceBetweenAtoms(const String& name1, const String& name2) const;
  double getDistanceBetweenAtoms(const String& name1, ProteinResiduePtr residue2, const String& name2) const;

  bool hasCAlphaAtom() const
    {return getCAlphaAtom();}

  bool hasOnlyCAlphaAtom() const
    {return atoms.size() && hasCAlphaAtom();}

  bool hasCompleteBackbone() const
    {return getCAlphaAtom() && getNitrogenAtom() && getCarbonAtom();}

  bool isCBetaAtomMissing() const
    {return aminoAcid != AminoAcidDictionary::glycine && !getCBetaAtom();}

  ProteinAtomPtr checkAndGetCBetaOrCAlphaAtom() const;

  void applyAffineTransform(const Matrix4& affineTransform) const;

  FeatureGeneratorPtr positionFeatures() const;

protected:
  AminoAcidDictionary::Type aminoAcid;
  std::vector<ProteinAtomPtr> atoms;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_RESIDUE_H_
