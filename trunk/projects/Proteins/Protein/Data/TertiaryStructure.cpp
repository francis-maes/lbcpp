/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructure.h            | Protein Tertiary Structure      |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 15:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "TertiaryStructure.h"
using namespace lbcpp;

TertiaryStructure::TertiaryStructure(size_t numResidues)
  : residues(new Vector(residueClass(), numResidues))
{
  thisClass = tertiaryStructureClass();
}

TertiaryStructurePtr TertiaryStructure::createFromCAlphaTrace(VectorPtr primaryStructure, CartesianPositionVectorPtr trace)
{
  size_t n = trace->size();
  jassert(primaryStructure && primaryStructure->size() == n);

  TertiaryStructurePtr res = new TertiaryStructure(n);
  for (size_t i = 0; i < n; ++i)
  {
    impl::Vector3 position = trace->getPosition(i);
    if (position.exists())
    {
      // create a residue with a single Ca atom
      ResiduePtr residue = new Residue((AminoAcidType)primaryStructure->getVariable(i).getInteger());
      AtomPtr atom = new Atom(T("CA"), T("C"));
      atom->setPosition(new Vector3(position));
      residue->addAtom(atom);
      res->setResidue(i, residue);
    }
  }
  return res;
}
/*
TertiaryStructurePtr TertiaryStructure::createFromBackbone(LabelSequencePtr aminoAcidSequence, ProteinBackboneBondSequencePtr backbone)
{
  size_t n = backbone->size();
  jassert(aminoAcidSequence && aminoAcidSequence->size() == n);

  impl::Matrix4 matrix = impl::Matrix4::identity;
  TertiaryStructurePtr res = new TertiaryStructure(n);
  bool previousBondExists = true;
  for (size_t i = 0; i < n; ++i)
  {
    ProteinBackboneBondPtr backboneResidue = backbone->getBond(i); 
    if (!backboneResidue->exists())
      continue;
    ResiduePtr residue = new Residue((AminoAcidDictionary::Type)aminoAcidSequence->getIndex(i));
    
    if (previousBondExists)
      residue->addAtom(new Atom(T("N"), T("N"), matrix.getTranslation()));

    impl::BondCoordinates bond = backboneResidue->getBond1();
    if (bond.exists())
    {
      bond.multiplyMatrix(matrix);
      residue->addAtom(new Atom(T("CA"), T("C"), matrix.getTranslation()));
    }

    bond = backboneResidue->getBond2();
    if (bond.exists())
    {
      bond.multiplyMatrix(matrix);
      residue->addAtom(new Atom(T("C"), T("C"), matrix.getTranslation()));
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
*/

VectorPtr TertiaryStructure::makeAminoAcidSequence() const
{
  size_t n = getNumResidues();
  jassert(n);
  VectorPtr res = new Vector(aminoAcidTypeEnumeration(), n);
  for (size_t i = 0; i < n; ++i)
    res->setVariable(i, getResidue(i)->getAminoAcidType());
  return res;
}

CartesianPositionVectorPtr TertiaryStructure::makeCAlphaTrace() const
{
  size_t n = getNumResidues();
  CartesianPositionVectorPtr res = new CartesianPositionVector(n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (!residue)
      continue;
    AtomPtr atom = residue->getCAlphaAtom();
    if (!atom)
    {
      Object::error(T("CartesianCoordinatesSequence::createCBetaTrace"),
          T("No C-alpha atom in residue ") + residue->getName() + T(" ") + lbcpp::toString(i + 1));
        return CartesianPositionVectorPtr();
    }
    res->setPosition(i, atom->getPosition());
  }
  return res;
}

CartesianPositionVectorPtr TertiaryStructure::makeCBetaTrace() const
{
  size_t n = getNumResidues();
  CartesianPositionVectorPtr res = new CartesianPositionVector(n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    AtomPtr atom = residue ? residue->checkAndGetCBetaOrCAlphaAtom() : AtomPtr();
    if (!atom)
      return CartesianPositionVectorPtr();
    res->setPosition(i, atom->getPosition());
  }
  return res;
}
/*
ProteinBackboneBondSequencePtr TertiaryStructure::makeBackbone() const
{
  size_t n = residues.size();

  // create backbone in cartesian coordinates
  CartesianCoordinatesSequencePtr backbone = new CartesianCoordinatesSequence(T("Backbone"), 3 * n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = residues[i];
    if (residue)
    {
      size_t j = i * 3;
      AtomPtr atom = residue->getNitrogenAtom();
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
      j + 2 < bondCoordinates->size() ? bondCoordinates->getCoordinates(j + 2) : impl::BondCoordinates());
    res->set(i, bond);
  }
  return res;
}*/

inline SymmetricMatrixPtr makeDistanceMatrix(const std::vector<impl::Vector3>& positions)
{
  size_t n = positions.size();

  SymmetricMatrixPtr res = new SymmetricMatrix(angstromDistanceType(), n);
  for (size_t i = 0; i < n; ++i)
  {
    res->setValue(i, i, Variable(0.0, angstromDistanceType()));
    impl::Vector3 position1 = positions[i];
    if (!position1.exists())
      continue;
    for (size_t j = i + 1; j < n; ++j)
    {
      impl::Vector3 position2 = positions[j];
      if (position2.exists())
        res->setValue(i, j, Variable((position1 - position2).l2norm(), angstromDistanceType()));
    }
  }
  return res;
}

SymmetricMatrixPtr TertiaryStructure::makeCAlphaDistanceMatrix() const
{
  size_t n = getNumResidues();
  std::vector<impl::Vector3> positions(n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    AtomPtr atom = residue ? residue->getCAlphaAtom() : AtomPtr();
    if (atom && atom->getPosition())
      positions[i] = atom->getPosition()->getValue();
  }
  return makeDistanceMatrix(positions);
}

SymmetricMatrixPtr TertiaryStructure::makeCBetaDistanceMatrix() const
{
  size_t n = getNumResidues();
  std::vector<impl::Vector3> positions(n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    AtomPtr atom = residue ? residue->checkAndGetCBetaOrCAlphaAtom() : AtomPtr();
    if (atom && atom->getPosition())
      positions[i] = atom->getPosition()->getValue();
  }
  return makeDistanceMatrix(positions);
}

bool TertiaryStructure::hasCompleteBackbone() const
{
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (residue && !residue->hasCompleteBackbone())
      return false;
  }
  return true;
}

bool TertiaryStructure::hasBackboneAndCBetaAtoms() const
{
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (residue && (!residue->hasCompleteBackbone() || residue->isCBetaAtomMissing()))
      return false;
  }
  return true;
}

bool TertiaryStructure::hasCAlphaAtoms() const
{
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (residue && !residue->hasCAlphaAtom())
      return false;
  }
  return true;
}

bool TertiaryStructure::isConsistent(String& failureReason) const
{
  bool res = true;
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    String position = T(" at position ") + lbcpp::toString(i + 1);
    ResiduePtr residue = getResidue(i);
    if (!residue)
      continue;

    if (!residue->getNumAtoms())
    {
      failureReason += T("Empty residue") + position + T("\n");
      res = false;
    }
    ResiduePtr nextResidue = i < n - 1 ? getResidue(i + 1) : ResiduePtr();
    
    if (residue->hasCompleteBackbone())
    {
      double d = residue->getDistanceBetweenAtoms(T("N"), T("CA")).getDouble();
      if (d < 1.0 || d > 2.0)
      {
        failureReason += T("Suspect N--CA distance: ") + lbcpp::toString(d) + position + T("\n");
        res = false;
      }

      d = residue->getDistanceBetweenAtoms(T("CA"), T("C")).getDouble();
      if (d < 1.0 || d > 2.0)
      {
        failureReason += T("Suspect CA--C distance: ") + lbcpp::toString(d) + position + T("\n");
        res = false;
      }
      if (nextResidue)
      {
        d = residue->getDistanceBetweenAtoms(T("C"), nextResidue, T("N")).getDouble();
        jassert(d);
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

void TertiaryStructure::pruneResiduesThatDoNotHaveCompleteBackbone()
{
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (residue && (!residue->hasCompleteBackbone() || residue->isCBetaAtomMissing()))
    {
      std::cout << "Prune incomplete residue " << i << std::endl;
      residue = ResiduePtr();
    }
  }
}

size_t TertiaryStructure::getNumSpecifiedResidues() const
{
  size_t n = getNumResidues();
  size_t res = 0;
  for (size_t i = 0; i < n; ++i)
    if (getResidue(i))
      ++res;
  return res;
}

impl::Matrix4 TertiaryStructure::superposeCAlphaAtoms(TertiaryStructurePtr targetStructure) const
{
  jassert(targetStructure);
  size_t n = getNumResidues();
  jassert(n && targetStructure->getNumResidues() == n);

  std::vector< std::pair<impl::Vector3, impl::Vector3> > pointPairs;
  pointPairs.reserve(n);
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue1 = getResidue(i);
    ResiduePtr residue2 = targetStructure->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    impl::Vector3 position1 = residue1->getAtomPosition(T("CA"));
    impl::Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    pointPairs.push_back(std::make_pair(position1, position2));
  }
  return impl::Matrix4::findAffineTransformToSuperposePoints(pointPairs);
}

double TertiaryStructure::computeCAlphaAtomsRMSE(TertiaryStructurePtr targetStructure) const
{
  impl::Matrix4 matrix = superposeCAlphaAtoms(targetStructure);

  size_t n = getNumResidues();
  size_t count = 0;
  double error = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue1 = getResidue(i);
    ResiduePtr residue2 = targetStructure->getResidue(i);
    if (!residue1 || !residue2)
      continue;
    impl::Vector3 position1 = residue1->getAtomPosition(T("CA"));
    impl::Vector3 position2 = residue2->getAtomPosition(T("CA"));
    if (!position1.exists() || !position2.exists())
      continue;
    
    double delta = (matrix.transformAffine(position1) - position2).l2norm();
    error += delta * delta;
    ++count;
  }
  return count ? sqrt(error / (double)count) : 0.0;
}

void TertiaryStructure::applyAffineTransform(const impl::Matrix4& affineTransform) const
{
  jassert(affineTransform.isAffine());
  size_t n = getNumResidues();
  for (size_t i = 0; i < n; ++i)
  {
    ResiduePtr residue = getResidue(i);
    if (residue)
      residue->applyAffineTransform(affineTransform);
  }
}

class TertiaryStructureClass : public DynamicClass
{
public:
  TertiaryStructureClass() : DynamicClass(T("TertiaryStructure"), objectClass())
    {addVariable(vectorClass(residueClass()), T("residues"));}

  virtual VariableValue create() const
    {return new TertiaryStructure();}
};

ClassPtr lbcpp::tertiaryStructureClass()
  {static TypeCache cache(T("TertiaryStructure")); return cache();}

void declareTertiaryStructureClasses()
{
  Class::declare(new TertiaryStructureClass());
}
