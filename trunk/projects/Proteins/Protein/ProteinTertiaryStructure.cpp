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
    ProteinResiduePtr residue = new ProteinResidue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    
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


// returns the matrix (rotation + translation) to transform points1 into points2
static Matrix4 superposeStructures(const std::vector< std::pair<Vector3, Vector3> >& pointPairs)
{
  size_t n = pointPairs.size();
  jassert(n);
  double invN = 1.0 / (double)n;

  // compute centroids
  Vector3 centroid1(0.0), centroid2(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    centroid1 += pointPairs[i].first;
    centroid2 += pointPairs[i].second;
  }
  centroid1 *= invN;
  centroid2 *= invN;

  // compute correlation matrix
  Matrix3 correlationMatrix = Matrix3::zero;
  for (size_t i = 0; i < n; ++i)
    correlationMatrix.addCorrelation(pointPairs[i].first - centroid1, pointPairs[i].second - centroid2);

  // make SVD decomposition
  Matrix3 u, v;
  Vector3 diag;
  bool ok = correlationMatrix.makeSVDDecomposition(u, diag, v);
  jassert(ok);
  if (!ok)
    return Matrix4::zero;

  // compute optimal rotation matrix
  Matrix3 rotation = v * u.transposed();

  // compute optimal translation
  Vector3 translation(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    translation += pointPairs[i].second;
    Vector3 p = rotation.transformAffine(pointPairs[i].first);
    translation -= p;
  }
  translation *= invN;

  return Matrix4(rotation, translation);
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
    ProteinResiduePtr residue1 = getResidue(i);
    ProteinResiduePtr residue2 = targetStructure->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    Vector3 position1 = residue1->getAtomPosition(T("CA"));
    Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    pointPairs.push_back(std::make_pair(position1, position2));
  }
  return superposeStructures(pointPairs);
}

double ProteinTertiaryStructure::computeCAlphaAtomsRMSE(ProteinTertiaryStructurePtr targetStructure) const
{
  Matrix4 matrix = superposeCAlphaAtoms(targetStructure);

  size_t n = size();
  size_t count = 0;
  double error = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr residue1 = getResidue(i);
    ProteinResiduePtr residue2 = targetStructure->getResidue(i);
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
