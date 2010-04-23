/*-----------------------------------------.---------------------------------.
| Filename: ProteinBackboneBondSequence.cpp| Protein Backbone Bonds          |
| Author  : Francis Maes                   |                                 |
| Started : 23/04/2010 13:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinBackboneBondSequence.h"
using namespace lbcpp;

/*
** ProteinBackboneBondSequence
*/
#if 0
static void computeDihedralAngles(ProteinResiduePtr previousResidue, ProteinResiduePtr residue, ProteinResiduePtr nextResidue, DihedralAngle& phi, DihedralAngle& psi)
{
  jassert(residue);

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

ProteinBackboneBondSequencePtr ProteinBackboneBondSequence::createDihedralAngles(ProteinTertiaryStructurePtr tertiaryStructure)
{
  size_t n =tertiaryStructure->size();
  ProteinBackboneBondSequencePtr res = new ProteinBackboneBondSequence(n);
  ProteinResiduePtr previousResidue;
  ProteinResiduePtr residue = tertiaryStructure->getResidue(0);
  for (size_t i = 0; i < n; ++i)
  {
    ProteinResiduePtr nextResidue = i < (n - 1) ? tertiaryStructure->getResidue(i + 1) : ProteinResiduePtr();
    DihedralAngle phi, psi;
    if (residue)
    {
      computeDihedralAngles(previousResidue, residue, nextResidue, phi, psi);
      res->setAnglesPair(i, phi, psi);
    }
    previousResidue = residue;
    residue = nextResidue;
  }
  return ProteinBackboneBondSequence();
}
#endif // 0

