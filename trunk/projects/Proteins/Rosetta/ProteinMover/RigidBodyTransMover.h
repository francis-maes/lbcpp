/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyTransMover.h          | RigidBodyTransMover             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:29:27         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_H_

# include "precompiled.h"
# include "../ProteinMover.h"

# undef T
#  include <core/kinematics/MoveMap.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <core/conformation/Conformation.hh>
//#  include <protocols/init.hh>
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

class RigidBodyTransMover;
typedef ReferenceCountedObjectPtr<RigidBodyTransMover> RigidBodyTransMoverPtr;

class RigidBodyTransMover: public RigidBodyMover
{
public:
  RigidBodyTransMover():RigidBodyMover(T("Rigid body translation mover"))
  {
  }

  /**
   * Instantiates a mover object that performs a translation of two residues following
   * the axis formed by them.
   *
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param magnitude a double specifying the magnitude of the translation.
   *
   * Note: The two residues must be separated by at least an other residue,
   * i.e. abs(index1-index2) >= 2.
   */
  RigidBodyTransMover(size_t indexResidueOne, size_t indexResidueTwo, double magnitude) :
    RigidBodyMover(T("Rigid body translation mover"), indexResidueOne, indexResidueTwo), magnitude(
        magnitude)
  {
  }

  RigidBodyTransMover(const RigidBodyTransMover& mover) :
        RigidBodyMover(T("Rigid body translation mover"), mover.indexResidueOne,
            mover.indexResidueTwo), magnitude(mover.magnitude)
  {
  }

  /**
   * Performs the translation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  virtual void move(core::pose::PoseOP& pose)
  {
    move(pose, indexResidueOne, indexResidueTwo, magnitude);
  }

  /**
   * Moves the two residues by a translation following the axis that is formed
   * by the two specified residues.
   *
   * @param pose the pose object to modify.
   * @param indexResidueOne an integer in [0, number residues - 1] that specifies the first
   * residue to modify,
   * @param indexResidueTwo an integer specifying the second residue in [0, number residues - 1],
   * @param magnitude a double specifying the magnitude of the translation.
   */
  static void move(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitude)
  {
      if (pose->n_residue() < 2)
        return;

      size_t firstResidue = indexResidueOne+1;
      size_t secondResidue = indexResidueTwo+1;

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
      foldTree.delete_jump_and_intervening_cutpoint(1);
      pose->fold_tree(foldTree);
  }

  /**
   * Sets the new increment for the distance.
   * @param amplitude the new value for the distance increment.
   */
  void setMagnitude(double newMagnitude)
  {
    magnitude = newMagnitude;
  }

  /**
   * Gets the current value for the distance increment.
   * @return the current distance increment.
   */
  double getMagnitude()
  {
    return magnitude;
  }

  RigidBodyTransMover operator=(const RigidBodyTransMover& mover)
  {
    RigidBodyTransMover copy;
    copy.indexResidueOne = mover.indexResidueOne;
    copy.indexResidueTwo = mover.indexResidueTwo;
    copy.magnitude = mover.magnitude;
    return copy;
  }

protected:
  friend class RigidBodyTransMoverClass;
  double magnitude;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_TRANS_MOVER_H_
