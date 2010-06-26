/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueAtoms.cpp        | ProteinObject Residue Atoms           |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinResidueAtoms.h"
using namespace lbcpp;

/*
** ProteinAtom
*/
String ProteinAtom::toString() const
{
  return getName() + T(" ") + position.toString() + T(" ")
    + lbcpp::toString(occupancy) + T(" ") + lbcpp::toString(temperatureFactor);
}

bool ProteinAtom::load(InputStream& istr)
{
  return NameableObject::load(istr) && lbcpp::read(istr, elementSymbol) &&
    lbcpp::read(istr, position) && lbcpp::read(istr, occupancy) &&
    lbcpp::read(istr, temperatureFactor);
}

void ProteinAtom::save(OutputStream& ostr) const
{
  NameableObject::save(ostr);
  lbcpp::write(ostr, elementSymbol);
  lbcpp::write(ostr, position);
  lbcpp::write(ostr, occupancy);
  lbcpp::write(ostr, temperatureFactor);
}

/*
** ProteinResidueAtoms
*/
String ProteinResidueAtoms::toString() const
{
  String res = T("Residue ") + AminoAcidDictionary::getThreeLettersCode(aminoAcid) + T(":");
  if (atoms.size())
  {
    res += T("\n");
    for (size_t i = 0; i < atoms.size(); ++i)
      res += T("  ") + atoms[i]->toString() + T("\n");
  }
  else
    res += T(" no atoms.\n");
  return res;
}

ProteinAtomPtr ProteinResidueAtoms::findAtomByName(const String& name) const
{
  for (size_t i = 0; i < atoms.size(); ++i)
    if (atoms[i]->getName() == name)
      return atoms[i];
  return ProteinAtomPtr();
}  

double ProteinResidueAtoms::getDistanceBetweenAtoms(const String& name1, const String& name2) const
{
  ProteinAtomPtr a1 = findAtomByName(name1);
  ProteinAtomPtr a2 = findAtomByName(name2);
  return a1 && a2 ? (a1->getPosition() - a2->getPosition()).l2norm() : DBL_MAX;
}

double ProteinResidueAtoms::getDistanceBetweenAtoms(const String& name1, ProteinResidueAtomsPtr residue2, const String& name2) const
{
  ProteinAtomPtr a1 = findAtomByName(name1);
  ProteinAtomPtr a2 = residue2->findAtomByName(name2);
  return a1 && a2 ? (a1->getPosition() - a2->getPosition()).l2norm() : DBL_MAX;
}

ProteinAtomPtr ProteinResidueAtoms::checkAndGetCBetaOrCAlphaAtom() const
{
  if (isCBetaAtomMissing())
  {
    Object::error(T("ProteinResidueAtoms::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-beta atom in residue ") + AminoAcidDictionary::getThreeLettersCode(getAminoAcid()));
    return ProteinAtomPtr();
  }
  if (!hasCAlphaAtom())
  {
    Object::error(T("ProteinResidueAtoms::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(getAminoAcid()));
    return ProteinAtomPtr();
  }
  ProteinAtomPtr atom = getCBetaAtom();
  if (!atom)
    atom = getCAlphaAtom();
  jassert(atom);
  return atom;
}

void ProteinResidueAtoms::applyAffineTransform(const Matrix4& affineTransform) const
{
  jassert(affineTransform.isAffine());
  for (size_t i = 0; i < atoms.size(); ++i)
  {
    Vector3 position = atoms[i]->getPosition();
    if (position.exists())
      atoms[i]->setPosition(affineTransform.transformAffine(position));
  }
}

bool ProteinResidueAtoms::load(InputStream& istr)
{
  int aminoAcidType;
  if (!lbcpp::read(istr, aminoAcidType))
    return false;
  if (aminoAcidType < 0 || aminoAcidType >= AminoAcidDictionary::unknown)
  {
    Object::error(T("ProteinResidueAtoms::load"), T("Invalid amino acid type ") + lbcpp::toString(aminoAcidType));
    return false;
  }
  aminoAcid = (AminoAcidDictionary::Type)aminoAcidType;
  return lbcpp::read(istr, atoms);
}

void ProteinResidueAtoms::save(OutputStream& ostr) const
{
  lbcpp::write(ostr, (int)aminoAcid);
  lbcpp::write(ostr, atoms);
}
