/*-----------------------------------------.---------------------------------.
| Filename: ShearMover.h                   | ShearMover                      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:29:53         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_H_

# include "precompiled.h"
# include "../ProteinMover.h"

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

namespace lbcpp
{

class ShearMover;
typedef ReferenceCountedObjectPtr<ShearMover> ShearMoverPtr;

class ShearMover : public ProteinMover
{
public:
  ShearMover()
    : ProteinMover() {}

  /**
   * Instantiates a mover object that performs a modification on the Phi angle of the
   * specified residue and on the Psi of the preceding residue.
   * angles of the specified residue.
   *
   * @param residue the index of the residue to modify (starting from 0 but if 0, one
   * angle will not change)
   * @param deltaPhi the increment of the Phi angle
   * @param deltaPsi the increment of the Psi angle
   */
  ShearMover(size_t residue, double deltaPhi, double deltaPsi)
    : ProteinMover(), residue(residue), deltaPhi(deltaPhi), deltaPsi(deltaPsi) {}

  /**
   * Performs the move on the pose specified by the parameters of the mover.
   * @param pose the pose to modify.
   */
  virtual void move(core::pose::PoseOP& pose)
    {move(pose, residue, deltaPhi, deltaPsi);}

  /**
   * Moves the angle Phi of the specified residue and the Psi angle of its predecessor.
   *
   * @param pose the pose object to modify.
   * @param residue the residue whose Phi angle is to be modified (starting form 0)
   * @parm deltaPhi the amount by which the Phi angle of the specified residue has
   * to be modified.
   * @parm deltaPsi the amount by which the Psi angle of the predecessor residue has
   * to be modified.
   */
  static void move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi)
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

  /**
   * Sets the new residue to modify.
   * @param newResidueNumber the new index of the residue to modify.
   */
  void setResidueIndex(size_t index)
    {residue = index;}

  /**
   * Gets the current index of the residue to modify.
   * @return the index of the current residue.
   */
  size_t getResidueIndex()
    {return residue;}

  /**
   * Sets the new increment for the PHI angle.
   * @param delta the new value for the PHI increment.
   */
  void setDeltaPhi(double delta)
    {deltaPhi = delta;}

  /**
   * Gets the current value for the PHI increment.
   * @return the current PHI increment.
   */
  double getDeltaPhi()
    {return deltaPhi;}

  /**
   * Sets the new increment for the PSI angle.
   * @param delta the new value for the PSI increment.
   */
  void setDeltaPsi(double delta)
    {deltaPsi = delta;}

  /**
   * Gets the current value for the PSI increment.
   * @return the current PSI increment.
   */
  double getDeltaPsi()
    {return deltaPsi;}

  virtual bool isEqual(const ProteinMoverPtr& mover, double tolerance)
  {
    if (mover.isInstanceOf<ShearMover> ())
    {
      double errorResidue = std::abs((double)residue
          - (double)mover.staticCast<ShearMover> ()->residue) / (double)residue;
      double errorPhi = std::abs((double)deltaPhi
          - (double)mover.staticCast<ShearMover> ()->deltaPhi) / (double)deltaPhi;
      double errorPsi = std::abs((double)deltaPsi
          - (double)mover.staticCast<ShearMover> ()->deltaPsi) / (double)deltaPsi;
      return ((errorResidue < tolerance) && (errorPhi < tolerance) && (errorPsi < tolerance));
    }
    else
      return false;
  }

protected:
  friend class ShearMoverClass;

  size_t residue;
  double deltaPhi;
  double deltaPsi;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_H_
