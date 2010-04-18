/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.cpp   | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinTertiaryStructure.h"
using namespace lbcpp;

/*
** ProteinCarbonTrace
*/
ProteinCarbonTracePtr ProteinCarbonTrace::createCAlphaTrace(ProteinTertiaryStructurePtr tertiaryStructure)
{
  size_t n = tertiaryStructure->size();
  ProteinCarbonTracePtr res = new ProteinCarbonTrace(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue = tertiaryStructure->getResidue(i);
    ProteinAtomPtr atom = residue->getCAlphaAtom();
    jassert(atom);
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

ProteinCarbonTracePtr ProteinCarbonTrace::createCBetaTrace(ProteinTertiaryStructurePtr tertiaryStructure)
{
  size_t n = tertiaryStructure->size();
  ProteinCarbonTracePtr res = new ProteinCarbonTrace(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue = tertiaryStructure->getResidue(i);
    ProteinAtomPtr atom = residue->getCBetaAtom();
    if (!atom)
    {
      jassert(residue->getAminoAcid() == AminoAcidDictionary::glycine);
      atom = residue->getCAlphaAtom();
    }
    jassert(atom);
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

/*
** ProteinAtom
*/
String ProteinAtom::toString() const
{
  return getName() + T(" ") + position.toString() + T(" ")
    + lbcpp::toString(occupancy) + T(" ") + lbcpp::toString(temperatureFactor);
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

/*
** ProteinTertiaryStructure
*/
ProteinTertiaryStructure::ProteinTertiaryStructure(size_t numResidues)
  : Sequence(T("TertiaryStructure")), residues(numResidues) {}

void ProteinTertiaryStructure::set(size_t index, ObjectPtr object)
{
  ProteinResiduePtr residue = object.dynamicCast<ProteinResidue>();
  jassert(residue);
  jassert(index < residues.size());
  residues[index] = residue;
}

ProteinTertiaryStructurePtr ProteinTertiaryStructure::createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, ProteinCarbonTracePtr trace)
{
  size_t n = trace->size();
  jassert(aminoAcidSequence && aminoAcidSequence->size() == n);

  ProteinTertiaryStructurePtr res = new ProteinTertiaryStructure(n);
  for (size_t i = 0; i < n; ++i)
  {
    // create a residue with a single Ca atom
    ProteinResiduePtr residue = new ProteinResidue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    ProteinAtomPtr atom = new ProteinAtom(T("CA"), T("C"));
    atom->setPosition(trace->getPosition(i));
    residue->addAtom(atom);
    res->setResidue(i, residue);
  }
  return res;
}
