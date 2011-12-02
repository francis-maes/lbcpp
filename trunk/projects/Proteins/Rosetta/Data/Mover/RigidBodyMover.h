/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyMover.h               | RigidBodyMover                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 mai 2011  18:08:06          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVER_RIGID_BODY_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVER_RIGID_BODY_MOVER_H_

# include "../ProteinMover.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/kinematics/FoldTree.hh>
#  include <core/conformation/Conformation.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

namespace lbcpp
{

class RigidBodyMover;
typedef ReferenceCountedObjectPtr<RigidBodyMover> RigidBodyMoverPtr;

class RigidBodyMover : public ProteinMover
{
public:
  RigidBodyMover()
    : ProteinMover() {}

  /**
   * Instantiates a mover object that performs a rotation around the axis formed by the
   * two specified residues followed by a translation along this axis.
   *
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param magnitude a double specifying the magnitude of the translation.
   * @param amplitude a double specifying the amplitude of the rotation.
   *
   * Note: The two residues must be separated by at least an other residue, i.e.
   * abs(index1-index2) >= 2.
   */
  RigidBodyMover(size_t indexResidueOne, size_t indexResidueTwo, double magnitude, double amplitude)
    : ProteinMover(), residues(new Pair(indexResidueOne, indexResidueTwo)),
      magnitude(magnitude), amplitude(amplitude) {}

  RigidBodyMover(const RigidBodyMover& mover)
    : ProteinMover(), residues(new Pair(mover.residues->getFirst(), mover.residues->getSecond())),
      magnitude(mover.magnitude), amplitude(mover.amplitude) {}

  /**
   * Performs the rotation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  virtual void move(core::pose::PoseOP& pose)
  {
    move(pose, residues->getFirst().getInteger(), residues->getSecond().getInteger(), magnitude,
        amplitude);
  }

  /**
   * Moves the two residues by a rotation around the axis that is formed
   * by the two specified residues and a translation along the same axis.
   * @param pose the pose object to modify.
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param magnitude a double specifying the magnitude of the translation.
   * @param amplitude a double specifying the amplitude of the rotation.
   */
  static void move(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo,
      double magnitudeTranslation, double amplitudeRotation)
  {
    if (isNumberValid(amplitudeRotation) && (amplitudeRotation != 0))
      applyRotation(pose, indexResidueOne, indexResidueTwo, amplitudeRotation);
    if (isNumberValid(magnitudeTranslation) && (magnitudeTranslation != 0))
      applyTranslation(pose, indexResidueOne, indexResidueTwo, magnitudeTranslation);
  }

  /**
   * Sets the new rotation angle.
   * @param amplitude the new rotation angle.
   */
  void setAmplitude(double newAmplitude)
    {amplitude = newAmplitude;}

  /**
   * Gets the current value for the rotation angle.
   * @return the current value for the rotation angle.
   */
  double getAmplitude()
    {return amplitude;}

  /**
   * Sets the new increment for the distance.
   * @param magnitude the new value for the distance increment.
   */
  void setMagnitude(double newMagnitude)
    {magnitude = newMagnitude;}

  /**
   * Gets the current value for the distance increment.
   * @return the current distance increment.
   */
  double getMagnitude()
    {return magnitude;}

  static void applyTranslation(core::pose::PoseOP& pose, size_t indexResidueOne,
      size_t indexResidueTwo, double magnitude)
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
    protocols::moves::RigidBodyTransMoverOP mover = new protocols::moves::RigidBodyTransMover(
        (*pose), 1);
    mover->step_size(magnitude);
    mover->apply((*pose));

    // Clear the jump
    foldTree.simple_tree(pose->n_residue());
    pose->fold_tree(foldTree);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

  static void applyRotation(core::pose::PoseOP& pose, size_t indexResidueOne,
      size_t indexResidueTwo, double amplitude)
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

  /**
   * Sets the new first residue to modify.
   * @param index the new index of the first residue to modify.
   */
  void setIndexResidueOne(size_t index)
    {residues->setFirst(index);}

  /**
   * Gets the current index of the first residue to modify.
   * @return the index of the current first residue.
   */
  size_t getIndexResidueOne()
    {return residues->getFirst().getInteger();}

  /**
   * Sets the new second residue to modify.
   * @param index the new index of the second residue to modify.
   */
  void setIndexResidueTwo(size_t index)
    {residues->setSecond(index);}

  /**
   * Gets the current index of the second residue to modify.
   * @return the index of the current second residue.
   */
  size_t getIndexResidueTwo()
    {return residues->getSecond().getInteger();}

  virtual bool isEqual(const ProteinMoverPtr& mover, double tolerance)
  {
    if (mover.isInstanceOf<RigidBodyMover> ())
    {
      size_t thisResidueOne = residues->getFirst().getInteger();
      size_t thisResidueTwo = residues->getSecond().getInteger();
      size_t moverResidueOne =
          mover.staticCast<RigidBodyMover> ()->residues->getFirst().getInteger();
      size_t moverResidueTwo =
          mover.staticCast<RigidBodyMover> ()->residues->getSecond().getInteger();
      double errorResidueOne = std::abs((double)thisResidueOne - (double)moverResidueOne)
          / (double)thisResidueOne;
      double errorResidueTwo = std::abs((double)thisResidueTwo - (double)moverResidueTwo)
          / (double)thisResidueTwo;
      double errorMagnitude = std::abs((double)magnitude
          - (double)mover.staticCast<RigidBodyMover> ()->magnitude) / (double)magnitude;
      double errorAmplitude = std::abs((double)amplitude
          - (double)mover.staticCast<RigidBodyMover> ()->amplitude) / (double)amplitude;
      return ((errorResidueOne < tolerance) && (errorResidueTwo < tolerance) && (errorMagnitude
          < tolerance) && (errorAmplitude < tolerance));
    }
    else
      return false;
  }

protected:
  friend class RigidBodyMoverClass;

  PairPtr residues;
  double magnitude; // for translation
  double amplitude; // for rotation
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_MOVER_H_
