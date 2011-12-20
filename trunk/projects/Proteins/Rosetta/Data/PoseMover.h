/*-----------------------------------------.---------------------------------.
| Filename:  PoseMover.h                   | PoseMover                       |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_POSE_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_POSE_MOVER_H_

# include "precompiled.h"
# include "../RosettaUtils.h"

namespace lbcpp
{

class PoseMover;
typedef ReferenceCountedObjectPtr<PoseMover> PoseMoverPtr;

class PoseMover: public Object
{
public:
  /**
   * Performs the perturbation on the object. Must be reimplemented in the inheriting
   * classes.
   * @param pose the pose to perturb.
   */
  virtual void move(core::pose::PoseOP& pose) = 0;

  virtual bool isEqual(const PoseMoverPtr& mover, double tolerance) = 0;

  virtual PoseMoverPtr getOpposite() = 0;

protected:
  friend class PoseMoverClass;
};

extern PoseMoverPtr phiPsiMover(size_t residue, double deltaPhi, double deltaPsi);
extern PoseMoverPtr shearMover(size_t residue, double deltaPhi, double deltaPsi);
extern PoseMoverPtr rigidBodyMover(size_t residue1, size_t residue2, double magnitude, double amplitude);

extern ClassPtr poseMoverClass;
extern ClassPtr phiPsiMoverClass;
extern ClassPtr rigidBodyMoverClass;
extern ClassPtr shearMoverClass;

extern EnumerationPtr poseMoverEnumerationEnumeration;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_POSE_MOVER_H_
