/*-----------------------------------------.---------------------------------.
| Filename: Residue.cpp                    | Residue Atoms                   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 15:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Residue.h"
using namespace lbcpp;

String Residue::toString() const
{
  String res = T("Residue ") + getName() + T(":");
  if (atoms->size())
  {
    res += T("\n");
    for (size_t i = 0; i < atoms->size(); ++i)
      res += T("  ") + atoms->getVariable(i).toString() + T("\n");
  }
  else
    res += T(" no atoms.\n");
  return res;
}

AtomPtr Residue::findAtomByName(const String& name) const
{
  for (size_t i = 0; i < atoms->size(); ++i)
  {
    AtomPtr atom = atoms->getObjectAndCast<Atom>(i);
    if (atom->getName() == name)
      return atom;
  }
  return AtomPtr();
}  

Variable Residue::getDistanceBetweenAtoms(const String& name1, const String& name2) const
{
  AtomPtr a1 = findAtomByName(name1);
  AtomPtr a2 = findAtomByName(name2);
  return a1 && a2
    ? Variable((a1->getPosition() - a2->getPosition()).l2norm(), angstromDistanceType())
    : Variable::missingValue(angstromDistanceType());
}

Variable Residue::getDistanceBetweenAtoms(const String& name1, ResiduePtr residue2, const String& name2) const
{
  AtomPtr a1 = findAtomByName(name1);
  AtomPtr a2 = residue2->findAtomByName(name2);
  return a1 && a2
    ? Variable((a1->getPosition() - a2->getPosition()).l2norm(), angstromDistanceType())
    : Variable::missingValue(angstromDistanceType());
}

AtomPtr Residue::checkAndGetCBetaOrCAlphaAtom() const
{
  if (isCBetaAtomMissing())
  {
    Object::error(T("Residue::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-beta atom in residue ") + getName());
    return AtomPtr();
  }
  if (!hasCAlphaAtom())
  {
    Object::error(T("Residue::checkAndGetCBetaOrCAlphaAtom"),
      T("No C-alpha atom in residue ") + getName());
    return AtomPtr();
  }
  AtomPtr atom = getCBetaAtom();
  if (!atom)
    atom = getCAlphaAtom();
  jassert(atom);
  return atom;
}

void Residue::applyAffineTransform(const impl::Matrix4& affineTransform) const
{
  jassert(affineTransform.isAffine());
  for (size_t i = 0; i < atoms->size(); ++i)
  {
    AtomPtr atom = atoms->getObjectAndCast<Atom>(i);
    impl::Vector3 position = atom->getPosition();
    if (position.exists())
      atom->setPosition(affineTransform.transformAffine(position));
  }
}

class ResidueClass : public DynamicClass
{
public:
  ResidueClass() : DynamicClass(T("Residue"), objectClass())
  {
    addVariable(aminoAcidTypeEnumeration(), T("aminoAcidType"));
    addVariable(vectorClass(atomClass()), T("atoms"));
  }

  virtual VariableValue create() const
    {return new Residue();}
};

ClassPtr lbcpp::residueClass()
  {static TypeCache cache(T("Residue")); return cache();}

void declareResidueClasses()
{
  Class::declare(new ResidueClass());
}
