/*-----------------------------------------.---------------------------------.
| Filename: RigidBodySpinMover.h           | RigidBodySpinMover              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:28:38         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_H_

# include "precompiled.h"
# include "../ProteinMover.h"

# undef T
#  include <core/kinematics/MoveMap.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <core/conformation/Conformation.hh>
#  include <protocols/init.hh>
#  include <protocols/moves/BackboneMover.hh>
#  include <protocols/moves/Mover.hh>
#  include <protocols/moves/MoverCreator.hh>
#  include <protocols/moves/MoverFactory.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  include <protocols/geometry/RB_geometry.hh>
#  include <core/kinematics/Jump.hh>
#  include <core/kinematics/Stub.hh>
# define T JUCE_T

namespace lbcpp
{

class RigidBodySpinMover;
typedef ReferenceCountedObjectPtr<RigidBodySpinMover> RigidBodySpinMoverPtr;
extern ClassPtr rigidBodySpinMoverClass;

class RigidBodySpinMover: public RigidBodyMover
{
public:

RigidBodySpinMover():RigidBodyMover(T("Rigid body spin mover"))
  {
  }

  /**
   * Instantiates a mover object that performs a rotation around the axis formed by the
   * two specified residues.
   *
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param amplitude a double specifying the amplitude of the rotation.
   *
   * Note: The two residues must be separated by at least an other residue, i.e.
   * abs(index1-index2) >= 2.
   */
  RigidBodySpinMover(size_t indexResidueOne, size_t indexResidueTwo, double amplitude) :
    RigidBodyMover(T("Rigid body spin mover"), indexResidueOne, indexResidueTwo), amplitude(
        amplitude)
  {
  }

  RigidBodySpinMover(const RigidBodySpinMover& mover) :
    RigidBodyMover(T("Rigid body spin mover"), mover.indexResidueOne, mover.indexResidueTwo),
        amplitude(mover.amplitude)
  {
  }

  /**
   * Performs the rotation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  void move(core::pose::PoseOP& pose)
  {
    move(pose, indexResidueOne, indexResidueTwo, amplitude);
  }

  /**
   * Moves the two residues by a rotation around the axis that is formed
   * by the two specified residues.
   * @param pose the pose object to modify.
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param amplitude a double specifying the amplitude of the rotation.
   */
  static void move(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double amplitude)
  {
    if (pose->n_residue() < 2)
      return;

    size_t firstResidue = indexResidueOne+1;
    size_t secondResidue = indexResidueTwo+1;

    // Set a jump
    core::kinematics::FoldTree foldTree = pose->fold_tree();
    int cutpointResidue = std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

    foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
    pose->fold_tree(foldTree);

    // Perturb the pose
    core::kinematics::Jump jumpToModify = pose->jump(1);
    core::kinematics::Stub firstStub = (pose->conformation()).upstream_jump_stub(1);
    core::kinematics::Stub secondStub = (pose->conformation()).downstream_jump_stub(1);

    // Create rotation axis and rotation center
    core::Vector upCentroids;
    core::Vector rotationCenter;
    protocols::geometry::centroids_by_jump((*pose), 1, upCentroids, rotationCenter);
    core::Vector rotationAxis = upCentroids - rotationCenter;

    // Apply rotation
    jumpToModify.set_rb_center(1, secondStub, rotationCenter);
    jumpToModify.rotation_by_axis(firstStub, rotationAxis, rotationCenter, amplitude);

    // Set new conformation and clear the jump
    pose->set_jump(1, jumpToModify);
    foldTree = pose->fold_tree();
    foldTree.delete_jump_and_intervening_cutpoint(1);
    pose->fold_tree(foldTree);
  }

  /**
   * Sets the new rotation angle.
   * @param amplitude the new rotation angle.
   */
  void setAmplitude(double amplitude)
  {
    this->amplitude = amplitude;
  }

  /**
   * Gets the current value for the rotation angle.
   * @return the current value for the rotation angle.
   */
  double getAmplitude()
  {
    return amplitude;
  }

  RigidBodySpinMover operator=(const RigidBodySpinMover& mover)
  {
    RigidBodySpinMover copy;
    copy.indexResidueOne = mover.indexResidueOne;
    copy.indexResidueTwo = mover.indexResidueTwo;
    copy.amplitude = mover.amplitude;
    return copy;
  }

protected:
  friend class RigidBodySpinMoverClass;
  double amplitude;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_SPIN_MOVER_H_
