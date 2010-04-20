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
** ProteinDihedralAngles
*/
static void computeDihedralAngles(ProteinResiduePtr previousResidue, ProteinResiduePtr residue, ProteinResiduePtr nextResidue, DihedralAngle& phi, DihedralAngle& psi)
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

  phi = previousCarbon ? DihedralAngle::compute(previousCarbon->getPosition(), nitrogenPos, calphaPos, carbonPos) : 2 * M_PI;
  psi = nextNitrogen ? DihedralAngle::compute(nitrogenPos, calphaPos, carbonPos, nextNitrogen->getPosition()) : 2 * M_PI;
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
    DihedralAngle phi, psi;
    computeDihedralAngles(previousResidue, residue, nextResidue, phi, psi);
    res->setAnglesPair(i, phi, psi);
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

LabelSequencePtr ProteinTertiaryStructure::createAminoAcidSequence() const
{
  size_t n = size();
  jassert(n);
  LabelSequencePtr res = new LabelSequence(T("AminoAcidSequence"), AminoAcidDictionary::getInstance(), n);
  for (size_t i = 0; i < n; ++i)
    res->setIndex(i, getResidue(i)->getAminoAcid());
  return res;
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
  Matrix4 currentReferential = Matrix4::identity();

  jassert(aminoAcidSequence && aminoAcidSequence->size() == dihedralAngles->size());

  ProteinTertiaryStructurePtr res = new ProteinTertiaryStructure(dihedralAngles->size());
  for (size_t i = 0; i < dihedralAngles->size(); ++i)
  {
    ProteinResiduePtr residue = new ProteinResidue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    residue->addAtom(new ProteinAtom(T("N"), T("N"), currentReferential.getTranslation()));

    currentReferential.translate(Vector3(1.46, 0.0, 0.0));
    residue->addAtom(new ProteinAtom(T("CA"), T("C"), currentReferential.getTranslation()));

    currentReferential.rotateAroundZAxis(1.203);
    currentReferential.rotateAroundXAxis(dihedralAngles->getPhi(i));
    currentReferential.translate(Vector3(1.53, 0.0, 0.0));
    residue->addAtom(new ProteinAtom(T("C"), T("C"), currentReferential.getTranslation()));
    
    currentReferential.rotateAroundZAxis(1.203);
    currentReferential.rotateAroundXAxis(dihedralAngles->getPsi(i));
    currentReferential.translate(Vector3(1.33, 0.0, 0.0));
    
    res->setResidue(i, residue);
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

bool ProteinTertiaryStructure::isConsistent(String& failureReason) const
{
  bool onlyCAlpha = hasOnlyCAlphaAtoms();

  for (size_t i = 0; i < residues.size(); ++i)
  {
    String position = T(" at position ") + lbcpp::toString(i + 1);
    ProteinResiduePtr residue = residues[i];
    if (!residue || !residue->getNumAtoms())
    {
      failureReason = T("Empty residue") + position;
      return false;
    }
    ProteinResiduePtr nextResidue = i < residues.size() - 1 ? residues[i + 1] : ProteinResiduePtr();
    
    if (!onlyCAlpha)
    {
      double d = residue->getDistanceBetweenAtoms(T("N"), T("CA"));
      if (d < 1.0 || d > 2.0)
      {
        failureReason = T("Suspect N--CA distance: ") + lbcpp::toString(d) + position;
        return false;
      }
      d = residue->getDistanceBetweenAtoms(T("CA"), T("C"));
      if (d < 1.0 || d > 2.0)
      {
        failureReason = T("Suspect CA--C distance: ") + lbcpp::toString(d) + position;
        return false;
      }
      if (nextResidue)
      {
        double d = residue->getDistanceBetweenAtoms(T("C"), nextResidue, T("N"));
        if (d < 1.0 || d > 2.0)
        {
          failureReason = T("Suspect C--N distance: ") + lbcpp::toString(d) + position;
          return false;
        }
      }
    }
  }
  return true;
}