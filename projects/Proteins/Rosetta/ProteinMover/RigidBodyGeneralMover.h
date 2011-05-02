/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyGeneralMover.h        | RigidBodyGeneralMover           |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:29:40         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_H_

# include "precompiled.h"
# include "../ProteinMover.h"
# include "RigidBodySpinMover.h"
# include "RigidBodyTransMover.h"

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

class RigidBodyGeneralMover;
typedef ReferenceCountedObjectPtr<RigidBodyGeneralMover> RigidBodyGeneralMoverPtr;

class RigidBodyGeneralMover: public RigidBodyMover
{
public:
  RigidBodyGeneralMover() :
    RigidBodyMover(T("Rigid body general mover"))
  {
  }

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
  RigidBodyGeneralMover(size_t indexResidueOne, size_t indexResidueTwo, double magnitude,
      double amplitude) :
    RigidBodyMover(T("Rigid body general mover"), indexResidueOne, indexResidueTwo), magnitude(
        magnitude), amplitude(amplitude)
  {
  }

  RigidBodyGeneralMover(const RigidBodyGeneralMover& mover) :
    RigidBodyMover(T("Rigid body general mover"), mover.indexResidueOne, mover.indexResidueTwo),
        magnitude(mover.magnitude), amplitude(mover.amplitude)
  {
  }

  /**
   * Performs the rotation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  void move(core::pose::PoseOP& pose)
  {
    move(pose, indexResidueOne, indexResidueTwo, magnitude, amplitude);
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
      double magnitude, double amplitude)
  {
    RigidBodySpinMover::move(pose, indexResidueOne, indexResidueTwo, amplitude);
    RigidBodyTransMover::move(pose, indexResidueOne, indexResidueTwo, magnitude);
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

  RigidBodyGeneralMover operator=(const RigidBodyGeneralMover& mover)
  {
    RigidBodyGeneralMover copy;
    copy.indexResidueOne = mover.indexResidueOne;
    copy.indexResidueTwo = mover.indexResidueTwo;
    copy.magnitude = mover.magnitude;
    copy.amplitude = mover.amplitude;
    return copy;
  }

protected:
  friend class RigidBodyGeneralMoverClass;
  double amplitude; // for rotation
  double magnitude; // for translation
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_H_
