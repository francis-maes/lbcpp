/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.cpp   | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinTertiaryStructure.h"
using namespace lbcpp;

#ifndef M_PI
# define M_PI       3.14159265358979323846
#endif // M_PI

class DihedralAngle
{
public:
  DihedralAngle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
    : angle(compute(a, b, c, d)) {}

  static double compute(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
  {    
    Vector3 ba = a - b;
    Vector3 bc = c - b;
    Vector3 dc = c - d;
    Vector3 normalB = ba.crossProduct(bc);
    Vector3 normalC = bc.crossProduct(dc);
    double res = normalB.angle(normalC);
    return bc.dotProduct(normalB.crossProduct(normalC)) >= 0 ? res : -res;
  }

private:
  double angle;
};

/*
** ProteinDihedralAngles
*/
ProteinDihedralAngles::ResidueInfo::ResidueInfo(ProteinResiduePtr previousResidue, ProteinResiduePtr residue, ProteinResiduePtr nextResidue)
{
  ProteinAtomPtr previousCarbon = previousResidue ? previousResidue->getCarbonAtom() : ProteinAtomPtr();
  ProteinAtomPtr nitrogen = residue->getNitrogenAtom();
  ProteinAtomPtr calpha = residue->getCAlphaAtom();
  ProteinAtomPtr carbon = residue->getCarbonAtom();
  ProteinAtomPtr nextNitrogen = nextResidue ? nextResidue->getNitrogenAtom() : ProteinAtomPtr();
  jassert(nitrogen && calpha && carbon && (!previousResidue || previousCarbon) && (!nextResidue || nextNitrogen));

  Vector3 nitrogenPos = nitrogen->getPosition();
  Vector3 calphaPos = calpha->getPosition();
  Vector3 carbonPos = carbon->getPosition();

  phiAngle = previousCarbon ? DihedralAngle::compute(previousCarbon->getPosition(), nitrogenPos, calphaPos, carbonPos) : 2 * M_PI;
  psiAngle = nextNitrogen ? DihedralAngle::compute(nitrogenPos, calphaPos, carbonPos, nextNitrogen->getPosition()) : 2 * M_PI;
}

ProteinDihedralAnglesPtr ProteinDihedralAngles::createDihedralAngles(ProteinTertiaryStructurePtr tertiaryStructure)
{
  size_t n = tertiaryStructure->size();

  ProteinDihedralAnglesPtr res = new ProteinDihedralAngles(n);

  ProteinResiduePtr previousResidue;
  ProteinResiduePtr residue = tertiaryStructure->getResidue(0);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr nextResidue = i < (n - 1) ? tertiaryStructure->getResidue(i + 1) : ProteinResiduePtr();
    res->angles[i] = ResidueInfo(previousResidue, residue, nextResidue);
    //std::cout << "Residue " << (i+1) << " phi = " << String(res->angles[i].phiAngle * 180 / M_PI, 1) << " psi = " << String(res->angles[i].psiAngle * 180 / M_PI, 1) << std::endl;
    previousResidue = residue;
    residue = nextResidue;
  }
  return res;
}

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
    if (!atom)
    {
      Object::error(T("ProteinCarbonTrace::createCBetaTrace"),
          T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
        return ProteinCarbonTracePtr();
    }
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
      if (residue->getAminoAcid() != AminoAcidDictionary::glycine)
      {
        Object::error(T("ProteinCarbonTrace::createCBetaTrace"),
          T("No C-beta atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
        return ProteinCarbonTracePtr();
      }
      atom = residue->getCAlphaAtom();
      if (!atom)
      {
        Object::error(T("ProteinCarbonTrace::createCBetaTrace"),
          T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
        return ProteinCarbonTracePtr();
      }
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

ProteinTertiaryStructurePtr ProteinTertiaryStructure::createFromDihedralAngles(LabelSequencePtr aminoAcidSequence, ProteinDihedralAnglesPtr dihedralAngles)
{
  ProteinTertiaryStructurePtr res = new ProteinTertiaryStructure(dihedralAngles->size());
  for (size_t i = 0; i < dihedralAngles->size(); ++i)
  {
    // todo
  }
  return res;
}

bool ProteinTertiaryStructure::hasOnlyCAlphaAtoms() const
{
  for (size_t i = 0; i < residues.size(); ++i)
    if (residues[i]->getNumAtoms() > 1 || residues[i]->getAtom(0)->getName() != T("CA"))
      return false;
  return true;
}
