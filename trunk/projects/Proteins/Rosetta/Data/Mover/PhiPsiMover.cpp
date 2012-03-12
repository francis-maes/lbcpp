/*-----------------------------------------.---------------------------------.
| Filename: PhiPsiMover.cpp                | PhiPsiMover source              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 28, 2012  12:06:38 PM      |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "PhiPsiMover.h"

using namespace lbcpp;

PhiPsiMover::PhiPsiMover() : PoseMover() {}
PhiPsiMover::PhiPsiMover(size_t residue, double deltaPhi, double deltaPsi)
  : PoseMover(), residue(residue), deltaPhi(deltaPhi), deltaPsi(deltaPsi) {}

void PhiPsiMover::move(core::pose::PoseOP& pose) const
  {move(pose, residue, deltaPhi, deltaPsi);}
void PhiPsiMover::move(PosePtr& pose) const
  {move(pose, residue, deltaPhi, deltaPsi);}

void PhiPsiMover::move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  if (std::isfinite(deltaPhi))
    pose->set_phi(residue + 1, pose->phi(residue + 1) + deltaPhi);
  if (std::isfinite(deltaPsi))
    pose->set_psi(residue + 1, pose->psi(residue + 1) + deltaPsi);
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
}

void PhiPsiMover::move(PosePtr& pose, int residue, double deltaPhi, double deltaPsi)
{
  jassert(isNumberValid(deltaPhi) && isNumberValid(deltaPsi));
  jassert((residue >= 0) && (residue < pose->getLength()));

  pose->setPhi(residue, pose->getPhi(residue) + deltaPhi);
  pose->setPsi(residue, pose->getPsi(residue) + deltaPsi);
}

void PhiPsiMover::setResidueIndex(size_t index)
  {residue = index;}
size_t PhiPsiMover::getResidueIndex() const
  {return residue;}

void PhiPsiMover::setDeltaPhi(double delta)
  {deltaPhi = delta;}
void PhiPsiMover::setDeltaPsi(double delta)
  {deltaPsi = delta;}
double PhiPsiMover::getDeltaPhi() const
  {return deltaPhi;}
double PhiPsiMover::getDeltaPsi() const
  {return deltaPsi;}

bool PhiPsiMover::isEqual(const PoseMoverPtr& mover) const
{
  if (!mover.isInstanceOf<PhiPsiMover> ())
    return false;

  double error = std::abs((double)residue - (double)mover.staticCast<PhiPsiMover> ()->residue) / (double)residue;
  error += std::abs((double)deltaPhi - (double)mover.staticCast<PhiPsiMover> ()->deltaPhi) / (double)deltaPhi;
  error += std::abs((double)deltaPsi - (double)mover.staticCast<PhiPsiMover> ()->deltaPsi) / (double)deltaPsi;
  return (error < LBCPP_POSEMOVER_TOLERANCE);
}

PoseMoverPtr PhiPsiMover::getOpposite() const
{
  PhiPsiMoverPtr temp = new PhiPsiMover(residue, -1.0 * deltaPhi, -1.0 * deltaPsi);
  return temp;
}
