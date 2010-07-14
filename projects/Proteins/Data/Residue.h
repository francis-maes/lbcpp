/*-----------------------------------------.---------------------------------.
| Filename: Residue.h                      | Residue Atoms                   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 15:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_RESIDUE_H_
# define LBCPP_PROTEIN_RESIDUE_H_

# include "AminoAcid.h"
# include "Atom.h"
# include "../Geometry/Matrix4.h"
# include <lbcpp/Data/Vector.h>

namespace lbcpp
{

class Residue;
typedef ReferenceCountedObjectPtr<Residue> ResiduePtr;

class Residue : public Object
{
public:
  Residue(AminoAcidType aminoAcidType)
    : aminoAcidType(aminoAcidType), atoms(new Vector(atomClass())) {}
  Residue() {}

  virtual String getName() const
    {return Variable(aminoAcidType, aminoAcidTypeEnumeration()).toString();}
  
  virtual String getThreeLettersCodeName() const
    {return AminoAcid::toThreeLettersCode(aminoAcidType);}

  AminoAcidType getAminoAcidType() const
    {return aminoAcidType;}

  void setAminoAcidType(AminoAcidType aminoAcidType)
    {this->aminoAcidType = aminoAcidType;}

  size_t getNumAtoms() const
    {return atoms->size();}

  AtomPtr getAtom(size_t index) const
    {jassert(index < atoms->size()); return atoms->getObjectAndCast<Atom>(index);}

  void addAtom(AtomPtr atom)
    {atoms->append(atom);}

  // central atom (back bone, first atom of the side chain)
  AtomPtr getCAlphaAtom() const
    {return findAtomByName(T("CA"));}

  // second carbon of the side chain
  AtomPtr getCBetaAtom() const
    {return findAtomByName(T("CB"));}

  // back bone
  AtomPtr getNitrogenAtom() const
    {return findAtomByName(T("N"));}

  // back bone
  AtomPtr getCarbonAtom() const
    {return findAtomByName(T("C"));}

  AtomPtr findAtomByName(const String& name) const;
  
  Vector3Ptr getAtomPosition(const String& name) const
    {AtomPtr atom = findAtomByName(name); return atom ? atom->getPosition() : Vector3Ptr();}

  Variable getDistanceBetweenAtoms(const String& name1, const String& name2) const;
  Variable getDistanceBetweenAtoms(const String& name1, ResiduePtr residue2, const String& name2) const;

  bool hasCAlphaAtom() const
    {return getCAlphaAtom();}

  bool hasOnlyCAlphaAtom() const
    {return atoms->size() && hasCAlphaAtom();}

  bool hasCompleteBackbone() const
    {return getCAlphaAtom() && getNitrogenAtom() && getCarbonAtom();}

  bool isCBetaAtomMissing() const
    {return aminoAcidType != glycine && !getCBetaAtom();}

  AtomPtr checkAndGetCBetaOrCAlphaAtom() const;

  void applyAffineTransform(const impl::Matrix4& affineTransform) const;

  // Object
  virtual String toString() const;

  virtual VariableReference getVariableReference(size_t index);

protected:
  AminoAcidType aminoAcidType;
  VectorPtr atoms;
};

extern ClassPtr residueClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_RESIDUE_H_
