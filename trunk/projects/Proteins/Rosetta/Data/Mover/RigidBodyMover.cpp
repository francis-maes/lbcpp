/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyMover.cpp             | RigidBodyMover source           |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 28, 2012  2:52:32 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "RigidBodyMover.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/kinematics/FoldTree.hh>
#  include <core/conformation/Conformation.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

using namespace lbcpp;

RigidBodyMover::RigidBodyMover() : PoseMover() {}
RigidBodyMover::RigidBodyMover(size_t indexResidueOne, size_t indexResidueTwo, double magnitude, double amplitude)
  : PoseMover(), residues(new Pair(indexResidueOne, indexResidueTwo)), magnitude(magnitude), amplitude(amplitude)
  {}
RigidBodyMover::RigidBodyMover(const RigidBodyMover& mover)
  : PoseMover(), residues(new Pair(mover.residues->getFirst(), mover.residues->getSecond())),
    magnitude(mover.magnitude), amplitude(mover.amplitude)
  {}

void RigidBodyMover::move(core::pose::PoseOP& pose) const
  {move(pose, residues->getFirst().getInteger(), residues->getSecond().getInteger(), magnitude, amplitude);}
void RigidBodyMover::move(PosePtr& pose) const
  {move(pose, residues->getFirst().getInteger(), residues->getSecond().getInteger(), magnitude, amplitude);}

void RigidBodyMover::move(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitudeTranslation,
    double amplitudeRotation)
{
  if (isNumberValid(amplitudeRotation) && (amplitudeRotation != 0))
    applyRotation(pose, indexResidueOne, indexResidueTwo, amplitudeRotation);
  if (isNumberValid(magnitudeTranslation) && (magnitudeTranslation != 0))
    applyTranslation(pose, indexResidueOne, indexResidueTwo, magnitudeTranslation);
}

void RigidBodyMover::move(PosePtr& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitudeTranslation,
    double amplitudeRotation)
{
  if (isNumberValid(amplitudeRotation) && (amplitudeRotation != 0))
    pose->applyRotation(indexResidueOne, indexResidueTwo, amplitudeRotation);
  if (isNumberValid(magnitudeTranslation) && (magnitudeTranslation != 0))
    pose->applyTranslation(indexResidueOne, indexResidueTwo, magnitudeTranslation);
}

void RigidBodyMover::setAmplitude(double newAmplitude)
  {amplitude = newAmplitude;}
double RigidBodyMover::getAmplitude()
  {return amplitude;}
void RigidBodyMover::setMagnitude(double newMagnitude)
  {magnitude = newMagnitude;}
double RigidBodyMover::getMagnitude()
  {return magnitude;}
void RigidBodyMover::setIndexResidueOne(size_t index)
  {residues->setFirst(index);}
size_t RigidBodyMover::getIndexResidueOne()
  {return residues->getFirst().getInteger();}
void RigidBodyMover::setIndexResidueTwo(size_t index)
  {residues->setSecond(index);}
size_t RigidBodyMover::getIndexResidueTwo()
  {return residues->getSecond().getInteger();}

bool RigidBodyMover::isEqual(const PoseMoverPtr& mover) const
{
  if (!mover.isInstanceOf<RigidBodyMover> ())
    return false;

  size_t thisResidueOne = residues->getFirst().getInteger();
  size_t thisResidueTwo = residues->getSecond().getInteger();
  size_t moverResidueOne = mover.staticCast<RigidBodyMover> ()->residues->getFirst().getInteger();
  size_t moverResidueTwo = mover.staticCast<RigidBodyMover> ()->residues->getSecond().getInteger();

  if ((thisResidueOne != moverResidueOne) || (thisResidueTwo != moverResidueTwo))
    return false;

  double error = std::abs(magnitude - mover.staticCast<RigidBodyMover> ()->magnitude) / std::abs(magnitude);
  error += std::abs(amplitude - mover.staticCast<RigidBodyMover> ()->amplitude) / std::abs(amplitude);

  return (error < LBCPP_POSEMOVER_TOLERANCE);
}

PoseMoverPtr RigidBodyMover::getOpposite() const
{
  RigidBodyMoverPtr temp = new RigidBodyMover(residues->getFirst().getInteger(), residues->getSecond().getInteger(), -1.0
      * magnitude, -1.0 * amplitude);
  return temp;
}

void RigidBodyMover::applyTranslation(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitude)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  jassert(pose->n_residue() > 2);

  size_t firstResidue = indexResidueOne + 1;
  size_t secondResidue = indexResidueTwo + 1;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = (int)std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  protocols::moves::RigidBodyTransMoverOP mover = new protocols::moves::RigidBodyTransMover((*pose), 1);
  mover->step_size(magnitude);
  mover->apply((*pose));

  // Clear the jump
  foldTree.simple_tree(pose->n_residue());
  pose->fold_tree(foldTree);
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
}

void RigidBodyMover::applyRotation(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double amplitude)
{
#ifdef LBCPP_PROTEIN_ROSETTA
  jassert(pose->n_residue() > 2);

  size_t firstResidue = indexResidueOne + 1;
  size_t secondResidue = indexResidueTwo + 1;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = (int)std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  core::kinematics::Jump jumpToModify = pose->jump(1);
  core::kinematics::Stub firstStub = (pose->conformation()).upstream_jump_stub(1);
  core::kinematics::Stub secondStub = (pose->conformation()).downstream_jump_stub(1);

  // Create rotation axis and rotation center
  core::Vector oneEnd = (pose->residue(firstResidue)).xyz("CA");
  core::Vector secondEnd = (pose->residue(secondResidue)).xyz("CA");
  core::Vector rotationAxis = oneEnd - secondEnd;

  // Apply rotation
  jumpToModify.set_rb_center(1, secondStub, secondEnd);
  jumpToModify.rotation_by_axis(firstStub, rotationAxis, secondEnd, (float)amplitude);

  // Set new conformation and clear the jump
  pose->set_jump(1, jumpToModify);
  foldTree = pose->fold_tree();
  foldTree.simple_tree(pose->n_residue());
  pose->fold_tree(foldTree);
#else
  jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
}
