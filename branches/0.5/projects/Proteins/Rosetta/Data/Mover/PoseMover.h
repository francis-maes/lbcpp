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
# include "../Pose.h"

# ifndef LBCPP_PROTEIN_ROSETTA
namespace utility {namespace pointer{
  template< typename T > class owning_ptr;
}; };

namespace core { namespace pose {
  class Pose;
  typedef utility::pointer::owning_ptr< Pose > PoseOP;
}; };
#endif // LBCPP_PROTEIN_ROSETTA

#define LBCPP_POSEMOVER_TOLERANCE 0.001

namespace lbcpp
{

class PoseMover;
typedef ReferenceCountedObjectPtr<PoseMover> PoseMoverPtr;

class PoseMover: public Object
{
public:

  virtual void move(core::pose::PoseOP& pose) const = 0;
  virtual void move(PosePtr& pose) const = 0;

  virtual bool isEqual(const PoseMoverPtr& mover) const = 0;
  virtual PoseMoverPtr getOpposite() const = 0;

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
