/*-----------------------------------------.---------------------------------.
| Filename: ShearMover.cpp                 | ShearMover source               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 28, 2012  2:33:42 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "ShearMover.h"

using namespace lbcpp;

ShearMover::ShearMover() : PoseMover() {}
ShearMover::ShearMover(size_t residue, double deltaPhi, double deltaPsi)
  : PoseMover(), residue(residue), deltaPhi(deltaPhi), deltaPsi(deltaPsi) {}

void ShearMover::move(core::pose::PoseOP& pose) const
  {move(pose, residue, deltaPhi, deltaPsi);}
void ShearMover::move(PosePtr& pose) const
  {move(pose, residue, deltaPhi, deltaPsi);}

void ShearMover::move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  if (std::isfinite(deltaPhi))
    pose->set_phi(residue + 1, pose->phi(residue + 1) + deltaPhi);
  if (std::isfinite(deltaPsi) && (residue != 0))
    pose->set_psi(residue, pose->psi(residue) + deltaPsi);
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
}

void ShearMover::move(PosePtr& pose, int residue, double deltaPhi, double deltaPsi)
{
  jassert(isNumberValid(deltaPhi) && isNumberValid(deltaPsi));
  jassert((residue >= 0) && (residue < pose->getLength()));

  pose->setPhi(residue, pose->getPhi(residue) + deltaPhi);
  if (residue != 0)
    pose->setPsi(residue, pose->getPsi(residue) + deltaPsi);
}

void ShearMover::setResidueIndex(size_t index)
  {residue = index;}
size_t ShearMover::getResidueIndex() const
  {return residue;}

void ShearMover::setDeltaPhi(double delta)
  {deltaPhi = delta;}
void ShearMover::setDeltaPsi(double delta)
  {deltaPsi = delta;}
double ShearMover::getDeltaPhi() const
  {return deltaPhi;}
double ShearMover::getDeltaPsi() const
  {return deltaPsi;}

bool ShearMover::isEqual(const PoseMoverPtr& mover) const
{
  if (!mover.isInstanceOf<ShearMover> ())
    return false;

  double error = std::abs((double)residue - (double)mover.staticCast<ShearMover> ()->residue) / (double)residue;
  error += std::abs((double)deltaPhi - (double)mover.staticCast<ShearMover> ()->deltaPhi) / (double)deltaPhi;
  error += std::abs((double)deltaPsi - (double)mover.staticCast<ShearMover> ()->deltaPsi) / (double)deltaPsi;
  return (error < LBCPP_POSEMOVER_TOLERANCE);
}

PoseMoverPtr ShearMover::getOpposite() const
{
  ShearMoverPtr temp = new ShearMover(residue, -1.0 * deltaPhi, -1.0 * deltaPsi);
  return temp;
}
