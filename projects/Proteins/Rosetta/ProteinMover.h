/*-----------------------------------------.---------------------------------.
| Filename:  ProteinMover.h                | ProteinMover                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_H_

# include "precompiled.h"
# include "RosettaUtils.h"

namespace lbcpp
{

class ProteinMover;
typedef ReferenceCountedObjectPtr<ProteinMover> ProteinMoverPtr;

class ProteinMover: public Object
{
public:
  /**
   * Performs the perturbation on the object. Must be reimplemented in the inheriting
   * classes.
   * @param pose the pose to perturb.
   */
  virtual void move(core::pose::PoseOP& pose)=0;

protected:
  friend class ProteinMoverClass;
};

extern ProteinMoverPtr phiPsiMover(size_t residue, double deltaPhi, double deltaPsi);
extern ProteinMoverPtr shearMover(size_t residue, double deltaPhi, double deltaPsi);
extern ProteinMoverPtr rigidBodyMover(size_t residue1, size_t residue2, double magnitude, double amplitude);

extern ClassPtr proteinMoverClass;
extern ClassPtr phiPsiMoverClass;
extern ClassPtr rigidBodyMoverClass;
extern ClassPtr shearMoverClass;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_H_
