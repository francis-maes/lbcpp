/*-----------------------------------------.---------------------------------.
| Filename: Residue.cpp                    | Protein Residue                 |
| Author  : Francis Maes                   |                                 |
| Started : 28/06/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

Residue::Residue(ProteinPtr protein, size_t position)
  : protein(protein), position(position)
{
  aminoAcid = aminoAcidCollection()->getElement((size_t)protein->getPrimaryStructure()->getVariable(position).getInteger());
  jassert(aminoAcid);
}

String Residue::toString() const
  {return protein->getName() + T("[") + String((int)position) + T("]");}

ResiduePtr Residue::getPrevious() const
  {return position > 0 ? protein->getResidue(position - 1) : ResiduePtr();}

ResiduePtr Residue::getNext() const
  {return position < protein->getLength() - 1 ? protein->getResidue(position + 1) : ResiduePtr();}

Variable Residue::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return aminoAcid;
  case 1: return getPrevious();
  case 2: return getNext();
  };
  return Variable();
}

class ResidueClass : public DynamicClass
{
public:
  ResidueClass() : DynamicClass(T("Residue"))
  {
    addVariable(aminoAcidCollection(), T("aminoAcid"));
    addVariable(TypePtr(this), T("previous"));
    addVariable(TypePtr(this), T("next"));
  }
};

void declareResidueClasses()
{
  Class::declare(new ResidueClass());
}

ClassPtr lbcpp::residueClass()
{
  static ClassPtr res = Class::get(T("Residue"));
  return res;
}
