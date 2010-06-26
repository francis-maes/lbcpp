/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructure.cpp   | ProteinObject Tertiary Structure      |
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
  ProteinResidueAtomsPtr residue = object.dynamicCast<ProteinResidueAtoms>();
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
    Vector3 position = trace->getPosition(i);
    if (position.exists())
    {
      // create a residue with a single Ca atom
      ProteinResidueAtomsPtr residue = new ProteinResidueAtoms((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
      ProteinAtomPtr atom = new ProteinAtom(T("CA"), T("C"));
      atom->setPosition(position);
      residue->addAtom(atom);
      res->setResidue(i, residue);
    }
  }
  return res;
}

ProteinTertiaryStructurePtr ProteinTertiaryStructure::createFromBackbone(LabelSequencePtr aminoAcidSequence, ProteinBackboneBondSequencePtr backbone)
{
  size_t n = backbone->size();
  jassert(aminoAcidSequence && aminoAcidSequence->size() == n);

  Matrix4 matrix = Matrix4::identity;
  ProteinTertiaryStructurePtr res = new ProteinTertiaryStructure(n);
  bool previousBondExists = true;
  for (size_t i = 0; i < n; ++i)
  {
    ProteinBackboneBondPtr backboneResidue = backbone->getBond(i); 
    if (!backboneResidue->exists())
      continue;
    ProteinResidueAtomsPtr residue = new ProteinResidueAtoms((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    
    if (previousBondExists)
      residue->addAtom(new ProteinAtom(T("N"), T("N"), matrix.getTranslation()));

    BondCoordinates bond = backboneResidue->getBond1();
    if (bond.exists())
    {
      bond.multiplyMatrix(matrix);
      residue->addAtom(new ProteinAtom(T("CA"), T("C"), matrix.getTranslation()));
    }

    bond = backboneResidue->getBond2();
    if (bond.exists())
    {
      bond.multiplyMatrix(matrix);
      residue->addAtom(new ProteinAtom(T("C"), T("C"), matrix.getTranslation()));
    }

    bond = backboneResidue->getBond3();
    if (bond.exists())
    {
      bond.multiplyMatrix(matrix);
      previousBondExists = true;
    }
    else
      previousBondExists = false;

    if (residue->getNumAtoms())
      res->setResidue(i, residue);
  }
  return res;
}

LabelSequencePtr ProteinTertiaryStructure::makeAminoAcidSequence() const
{
  size_t n = size();
  jassert(n);
  LabelSequencePtr res = new LabelSequence(T("AminoAcidSequence"), AminoAcidDictionary::getInstance(), n);
  for (size_t i = 0; i < n; ++i)
    res->setIndex(i, getResidue(i)->getAminoAcid());
  return res;
}

CartesianCoordinatesSequencePtr ProteinTertiaryStructure::makeCAlphaTrace() const
{
  size_t n = size();
  CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = getResidue(i);
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

CartesianCoordinatesSequencePtr ProteinTertiaryStructure::makeCBetaTrace() const
{
  size_t n = size();
  CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(T("CAlphaTrace"), n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = getResidue(i);
    ProteinAtomPtr atom = residue ? residue->checkAndGetCBetaOrCAlphaAtom() : ProteinAtomPtr();
    if (!atom)
      return CartesianCoordinatesSequencePtr();
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

ProteinBackboneBondSequencePtr ProteinTertiaryStructure::makeBackbone() const
{
  size_t n = residues.size();

  // create backbone in cartesian coordinates
  CartesianCoordinatesSequencePtr backbone = new CartesianCoordinatesSequence(T("Backbone"), 3 * n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = residues[i];
    if (residue)
    {
      size_t j = i * 3;
      ProteinAtomPtr atom = residue->getNitrogenAtom();
      if (atom)
        backbone->setPosition(j, atom->getPosition());
      atom = residue->getCAlphaAtom();
      if (atom)
        backbone->setPosition(j + 1, atom->getPosition());
      atom = residue->getCarbonAtom();
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

inline ScoreSymmetricMatrixPtr makeDistanceMatrix(const String& name, const std::vector<Vector3>& positions)
{
  size_t n = positions.size();

  ScoreSymmetricMatrixPtr res = new ScoreSymmetricMatrix(name, n);
  for (size_t i = 0; i < n; ++i)
  {
    res->setScore(i, i, 0.0);
    Vector3 position1 = positions[i];
    if (!position1.exists())
      continue;
    for (size_t j = i + 1; j < n; ++j)
    {
      Vector3 position2 = positions[j];
      if (position2.exists())
        res->setScore(i, j, (position1 - position2).l2norm());
    }
  }
  return res;
}

ScoreSymmetricMatrixPtr ProteinTertiaryStructure::makeCAlphaDistanceMatrix() const
{
  size_t n = size();
  std::vector<Vector3> positions(n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = getResidue(i);
    ProteinAtomPtr atom = residue ? residue->getCAlphaAtom() : ProteinAtomPtr();
    if (atom)
      positions[i] = atom->getPosition();
  }
  return makeDistanceMatrix(T("ResidueResidueDistanceMatrixCa"), positions);
}

ScoreSymmetricMatrixPtr ProteinTertiaryStructure::makeCBetaDistanceMatrix() const
{
  size_t n = size();
  std::vector<Vector3> positions(n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = getResidue(i);
    ProteinAtomPtr atom = residue ? residue->checkAndGetCBetaOrCAlphaAtom() : ProteinAtomPtr();
    if (atom)
      positions[i] = atom->getPosition();
  }
  return makeDistanceMatrix(T("ResidueResidueDistanceMatrixCb"), positions);
}

bool ProteinTertiaryStructure::hasCompleteBackbone() const
{
  for (size_t i = 0; i < residues.size(); ++i)
  {
    ProteinResidueAtomsPtr residue = residues[i];
    if (residue && !residue->hasCompleteBackbone())
      return false;
  }
  return true;
}

bool ProteinTertiaryStructure::hasBackboneAndCBetaAtoms() const
{
  for (size_t i = 0; i < residues.size(); ++i)
  {
    ProteinResidueAtomsPtr residue = residues[i];
    if (residue && (!residue->hasCompleteBackbone() || residue->isCBetaAtomMissing()))
      return false;
  }
  return true;
}

bool ProteinTertiaryStructure::hasCAlphaAtoms() const
{
  for (size_t i = 0; i < residues.size(); ++i)
  {
    ProteinResidueAtomsPtr residue = residues[i];
    if (residue && !residue->hasCAlphaAtom())
      return false;
  }
  return true;
}

bool ProteinTertiaryStructure::isConsistent(String& failureReason) const
{
  bool res = true;
  for (size_t i = 0; i < residues.size(); ++i)
  {
    String position = T(" at position ") + lbcpp::toString(i + 1);
    ProteinResidueAtomsPtr residue = residues[i];
    if (!residue)
      continue;

    if (!residue->getNumAtoms())
    {
      failureReason += T("Empty residue") + position + T("\n");
      res = false;
    }
    ProteinResidueAtomsPtr nextResidue = i < residues.size() - 1 ? residues[i + 1] : ProteinResidueAtomsPtr();
    
    if (residue->hasCompleteBackbone())
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
      residues[i] = ProteinResidueAtomsPtr();
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

Matrix4 ProteinTertiaryStructure::superposeCAlphaAtoms(ProteinTertiaryStructurePtr targetStructure) const
{
  jassert(targetStructure);
  size_t n = size();
  jassert(n && targetStructure->size() == n);

  std::vector< std::pair<Vector3, Vector3> > pointPairs;
  pointPairs.reserve(n);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue1 = getResidue(i);
    ProteinResidueAtomsPtr residue2 = targetStructure->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    Vector3 position1 = residue1->getAtomPosition(T("CA"));
    Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    pointPairs.push_back(std::make_pair(position1, position2));
  }
  return Matrix4::findAffineTransformToSuperposePoints(pointPairs);
}

double ProteinTertiaryStructure::computeCAlphaAtomsRMSE(ProteinTertiaryStructurePtr targetStructure) const
{
  Matrix4 matrix = superposeCAlphaAtoms(targetStructure);

  size_t n = size();
  size_t count = 0;
  double error = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue1 = getResidue(i);
    ProteinResidueAtomsPtr residue2 = targetStructure->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    Vector3 position1 = residue1->getAtomPosition(T("CA"));
    Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    double delta = (matrix.transformAffine(position1) - position2).l2norm();
    error += delta * delta;
    ++count;
  }
  return count ? sqrt(error / (double)count) : 0.0;
}

void ProteinTertiaryStructure::applyAffineTransform(const Matrix4& affineTransform) const
{
  jassert(affineTransform.isAffine());
  size_t n = size();
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResidueAtomsPtr residue = getResidue(i);
    if (residue)
      residue->applyAffineTransform(affineTransform);
  }
}

bool ProteinTertiaryStructure::load(InputStream& istr)
{
  size_t numResidues;
  if (!Sequence::load(istr) || !lbcpp::read(istr, numResidues))
    return false;

  // compatibility loading: ProteinResidueAtoms was initially called ProteinResidue
  residues.clear();
  residues.reserve(numResidues);
  for (size_t i = 0; i < numResidues; ++i)
  {
    String className;
    if (!lbcpp::read(istr, className))
      return false;
    ProteinResidueAtomsPtr residue;
    if (className != T("__null__"))
    {
      jassert(className == T("ProteinResidue") || className == T("ProteinResidueAtoms"));
      residue = new ProteinResidueAtoms();
      if (!residue->load(istr))
        return false;
    }
    residues.push_back(residue);
  }
  return true;
}
