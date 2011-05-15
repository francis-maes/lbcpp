/*-----------------------------------------.---------------------------------.
| Filename: PhiPsiMover.h                  | PhiPsiMover                     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:28:24         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_H_

# include "precompiled.h"
# include "../ProteinMover.h"

namespace lbcpp
{

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/kinematics/MoveMap.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <core/conformation/Conformation.hh>
#  include <protocols/moves/BackboneMover.hh>
#  include <protocols/moves/Mover.hh>
#  include <protocols/moves/MoverCreator.hh>
#  include <protocols/moves/MoverFactory.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  include <protocols/geometry/RB_geometry.hh>
#  include <core/kinematics/Jump.hh>
#  include <core/kinematics/Stub.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

class PhiPsiMover;
typedef ReferenceCountedObjectPtr<PhiPsiMover> PhiPsiMoverPtr;

class PhiPsiMover : public ProteinMover
{
public:
  PhiPsiMover()
    : ProteinMover()
  {
  }

  /**
   * Instantiates a mover object that performs a modification on the Phi and Psi
   * angles of the specified residue.
   *
   * @param residue the index of the residue to modify (starting from 0)
   * @param deltaPhi the increment of the Phi angle
   * @param deltaPsi the increment of the Psi angle
   */
  PhiPsiMover(size_t residue, double deltaPhi, double deltaPsi)
    : ProteinMover(), residue(residue), deltaPhi(deltaPhi), deltaPsi(deltaPsi)
  {
  }

  /**
   * Performs the move on the pose specified by the parameters of the mover.
   * @param pose the pose to modify.
   */
  virtual void move(core::pose::PoseOP& pose)
  {
    move(pose, residue, deltaPhi, deltaPsi);
  }

  /**
   * Moves the angles Phi and Psi of the residue specified by an amount specified.
   *
   * @param pose the pose object to modify.
   * @param residue the residue whose angles are to be modified.
   * @parm deltaPhi the amount by which the Phi angle of the specified residue has
   * to be modified.
   * @parm deltaPsi the amount by which the Psi angle of the specified residue has
   * to be modified.
   */
  static void move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    pose->set_phi(residue + 1, pose->phi(residue + 1) + deltaPhi);
    pose->set_psi(residue + 1, pose->psi(residue + 1) + deltaPsi);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

  /**
   * Sets the new residue to modify.
   * @param newResidueNumber the new index of the residue to modify.
   */
  void setResidueIndex(size_t index)
  {
    residue = index;
  }

  /**
   * Gets the current index of the residue to modify.
   * @return the index of the current residue.
   */
  size_t getResidueIndex()
  {
    return residue;
  }

  /**
   * Sets the new increment for the PHI angle.
   * @param delta the new value for the PHI increment.
   */
  void setDeltaPhi(double delta)
  {
    deltaPhi = delta;
  }

  /**
   * Gets the current value for the PHI increment.
   * @return the current PHI increment.
   */
  double getDeltaPhi()
  {
    return deltaPhi;
  }

  /**
   * Sets the new increment for the PSI angle.
   * @param delta the new value for the PSI increment.
   */
  void setDeltaPsi(double delta)
  {
    deltaPsi = delta;
  }

  /**
   * Gets the current value for the PSI increment.
   * @return the current PSI increment.
   */
  double getDeltaPsi()
  {
    return deltaPsi;
  }

protected:
  friend class PhiPsiMoverClass;

  size_t residue;
  double deltaPhi;
  double deltaPsi;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_H_
