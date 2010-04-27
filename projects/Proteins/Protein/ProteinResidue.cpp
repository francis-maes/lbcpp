/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidue.cpp             | Protein Residue                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinResidue.h"
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
** ProteinResidue
*/
String ProteinResidue::toString() const
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

ProteinAtomPtr ProteinResidue::findAtomByName(const String& name) const
{
  for (size_t i = 0; i < atoms.size(); ++i)
    if (atoms[i]->getName() == name)
      return atoms[i];
  return ProteinAtomPtr();
}  

double ProteinResidue::getDistanceBetweenAtoms(const String& name1, const String& name2) const
{
  ProteinAtomPtr a1 = findAtomByName(name1);
  ProteinAtomPtr a2 = findAtomByName(name2);
  return a1 && a2 ? (a1->getPosition() - a2->getPosition()).l2norm() : DBL_MAX;
}

double ProteinResidue::getDistanceBetweenAtoms(const String& name1, ProteinResiduePtr residue2, const String& name2) const
{
  ProteinAtomPtr a1 = findAtomByName(name1);
  ProteinAtomPtr a2 = residue2->findAtomByName(name2);
  return a1 && a2 ? (a1->getPosition() - a2->getPosition()).l2norm() : DBL_MAX;
}

ProteinAtomPtr ProteinResidue::checkAndGetCBetaOrCAlphaAtom() const
{
  if (isCBetaAtomMissing())
  {
    Object::error(T("ProteinResidue::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-beta atom in residue ") + AminoAcidDictionary::getThreeLettersCode(getAminoAcid()));
    return ProteinAtomPtr();
  }
  if (!hasCAlphaAtom())
  {
    Object::error(T("ProteinResidue::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(getAminoAcid()));
    return ProteinAtomPtr();
  }
  ProteinAtomPtr atom = getCBetaAtom();
  if (!atom)
    atom = getCAlphaAtom();
  jassert(atom);
  return atom;
}

bool ProteinResidue::load(InputStream& istr)
{
  int aminoAcidType;
  if (!lbcpp::read(istr, aminoAcidType))
    return false;
  if (aminoAcidType < 0 || aminoAcidType >= 20)
  {
    Object::error(T("ProteinResidue::load"), T("Invalid amino acid type ") + lbcpp::toString(aminoAcidType));
    return false;
  }
  aminoAcid = (AminoAcidDictionary::Type)aminoAcidType;
  return lbcpp::read(istr, atoms);
}

void ProteinResidue::save(OutputStream& ostr) const
{
  lbcpp::write(ostr, (int)aminoAcid);
  lbcpp::write(ostr, atoms);
}
