/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.cpp   | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinTertiaryStructure.h"
using namespace lbcpp;

ProteinTertiaryStructure::ProteinTertiaryStructure(size_t numResidues)
  : Sequence(T("TertiaryStructure")), residues(numResidues) {}

void ProteinTertiaryStructure::set(size_t index, ObjectPtr object)
{
  ProteinResiduePtr residue = object.dynamicCast<ProteinResidue>();
  jassert(residue);
  jassert(index < residues.size());
  residues[index] = residue;
}

ProteinTertiaryStructurePtr ProteinTertiaryStructure::createFromCAlphaTrace(LabelSequencePtr aminoAcidSequence, CartesianCoordinatesSequencePtr trace)
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

ProteinTertiaryStructurePtr ProteinTertiaryStructure::createFromDihedralAngles(LabelSequencePtr aminoAcidSequence, ProteinBackboneBondSequencePtr dihedralAngles)
{
  Matrix4 currentReferential = Matrix4::identity;

  size_t n = dihedralAngles->size();
  jassert(aminoAcidSequence && aminoAcidSequence->size() == n);

  ProteinTertiaryStructurePtr res = new ProteinTertiaryStructure(n);
/*  for (size_t i = 0; i < dihedralAngles->size(); ++i)
  {
    ProteinResiduePtr residue = new ProteinResidue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    residue->addAtom(new ProteinAtom(T("N"), T("N"), currentReferential.getTranslation()));
    //std::cout << "N at " << lbcpp::toString(currentReferential.getTranslation()) << std::endl;

    currentReferential.translate(Vector3(1.46, 0.0, 0.0));
    if (i > 0)
      currentReferential.rotateAroundXAxis(dihedralAngles->getPhi(i));
    currentReferential.rotateAroundZAxis(1.216);
    residue->addAtom(new ProteinAtom(T("CA"), T("C"), currentReferential.getTranslation()));
    //std::cout << "CA at " << lbcpp::toString(currentReferential.getTranslation()) << std::endl;
    
    currentReferential.translate(Vector3(1.53, 0.0, 0.0));
    if (i < n - 1)
      currentReferential.rotateAroundXAxis(dihedralAngles->getPsi(i));
    currentReferential.rotateAroundZAxis(1.098);
    residue->addAtom(new ProteinAtom(T("C"), T("C"), currentReferential.getTranslation()));
    //std::cout << "C at " << lbcpp::toString(currentReferential.getTranslation()) << std::endl;
    
    currentReferential.translate(Vector3(1.33, 0.0, 0.0));
    currentReferential.rotateAroundXAxis(M_PI); // omega
    currentReferential.rotateAroundZAxis(1.033);
    res->setResidue(i, residue);
  }*/
  return res;
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

CartesianCoordinatesSequencePtr ProteinTertiaryStructure::createCAlphaTrace() const
{
  size_t n = size();
  CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue = getResidue(i);
    if (!residue)
      continue; // FIXME: CartesianCoordinatesSequencePtr must support NULL values
    ProteinAtomPtr atom = residue->getCAlphaAtom();
    if (!atom)
    {
      Object::error(T("CartesianCoordinatesSequence::createCBetaTrace"),
          T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
        return CartesianCoordinatesSequencePtr();
    }
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

CartesianCoordinatesSequencePtr ProteinTertiaryStructure::createCBetaTrace() const
{
  size_t n = size();
  CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue = getResidue(i);
    if (!residue)
      continue; // FIXME: CartesianCoordinatesSequencePtr must support NULL values
    if (residue->isCBetaAtomMissing())
    {
      Object::error(T("CartesianCoordinatesSequence::createCBetaTrace"),
        T("No C-beta atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
      return CartesianCoordinatesSequencePtr();
    }
    if (!residue->hasCAlphaAtom())
    {
      Object::error(T("CartesianCoordinatesSequence::createCBetaTrace"),
        T("No C-alpha atom in residue ") + AminoAcidDictionary::getThreeLettersCode(residue->getAminoAcid()) + T(" ") + lbcpp::toString(i + 1));
      return CartesianCoordinatesSequencePtr();
    }
    ProteinAtomPtr atom = residue->getCBetaAtom();
    if (!atom)
      atom = residue->getCAlphaAtom();
    jassert(atom);
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

ProteinBackboneBondSequencePtr ProteinTertiaryStructure::createBackbone() const
{
  size_t n = residues.size();

  // create backbone in cartesian coordinates
  CartesianCoordinatesSequencePtr backbone = new CartesianCoordinatesSequence(T("Backbone"), 3 * n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue = residues[i];
    if (residue)
    {
      size_t j = i * 3;
      ProteinAtomPtr atom = residue->getNitrogenAtom();
      if (atom)
        backbone->setPosition(j, atom->getPosition());
      atom = residue->getCAlphaAtom();
      if (atom)
        backbone->setPosition(j + 1, atom->getPosition());
      atom = residue->getNitrogenAtom();
      if (atom)
        backbone->setPosition(j + 2, atom->getPosition());
    }
  }

  // convert to bond coordinates
  BondCoordinatesSequencePtr bondCoordinates = new BondCoordinatesSequence(T("Backbone"), backbone);
  jassert(bondCoordinates->size() == 3 * n - 1);

  // create the protein backbone bond sequence
  ProteinBackboneBondSequencePtr res = new ProteinBackboneBondSequence(n);
  for (size_t i = 0; i < n; ++i)
  {
    size_t j = i * 3;
    ProteinBackboneBondPtr bond = new ProteinBackboneBond(
      bondCoordinates->getCoordinates(j),
      bondCoordinates->getCoordinates(j + 1),
      j + 2 < bondCoordinates->size() ? bondCoordinates->getCoordinates(j + 2) : BondCoordinates());
    res->set(i, bond);
  }
  return res;
}

bool ProteinTertiaryStructure::hasOnlyCAlphaAtoms() const
{
  for (size_t i = 0; i < residues.size(); ++i)
  {
    ProteinResiduePtr residue = residues[i];
    if (residue && (residue->getNumAtoms() > 1 || residue->getAtom(0)->getName() != T("CA")))
      return false;
  }
  return true;
}

bool ProteinTertiaryStructure::isConsistent(String& failureReason) const
{
  bool onlyCAlpha = hasOnlyCAlphaAtoms();

  bool res = true;
  for (size_t i = 0; i < residues.size(); ++i)
  {
    String position = T(" at position ") + lbcpp::toString(i + 1);
    ProteinResiduePtr residue = residues[i];
    if (!residue)
      continue;

    if (!residue->getNumAtoms())
    {
      failureReason += T("Empty residue") + position + T("\n");
      res = false;
    }
    ProteinResiduePtr nextResidue = i < residues.size() - 1 ? residues[i + 1] : ProteinResiduePtr();
    
    if (!onlyCAlpha)
    {
      double d = residue->getDistanceBetweenAtoms(T("N"), T("CA"));
      if (d < 1.0 || d > 2.0)
      {
        failureReason += T("Suspect N--CA distance: ") + lbcpp::toString(d) + position + T("\n");
        res = false;
      }
      d = residue->getDistanceBetweenAtoms(T("CA"), T("C"));
      if (d < 1.0 || d > 2.0)
      {
        failureReason += T("Suspect CA--C distance: ") + lbcpp::toString(d) + position + T("\n");
        res = false;
      }
      if (nextResidue)
      {
        double d = residue->getDistanceBetweenAtoms(T("C"), nextResidue, T("N"));
        if (d < 1.0 || d > 2.0)
        {
          failureReason += T("Suspect C--N distance: ") + lbcpp::toString(d) + position + T("\n");
          res = false;
        }
      }
    }
  }
  return res;
}

void ProteinTertiaryStructure::pruneResiduesThatDoNotHaveCompleteBackbone()
{
  for (size_t i = 0; i < residues.size(); ++i)
    if (residues[i] && (!residues[i]->hasCompleteBackbone() || residues[i]->isCBetaAtomMissing()))
    {
      std::cout << "Prune incomplete residue " << i << std::endl;
      residues[i] = ProteinResiduePtr();
    }
}

size_t ProteinTertiaryStructure::getNumSpecifiedResidues() const
{
  size_t res = 0;
  for (size_t i = 0; i < residues.size(); ++i)
    if (residues[i])
      ++res;
  return res;
}
