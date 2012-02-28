/*-----------------------------------------.---------------------------------.
| Filename: ShearMover.h                   | ShearMover                      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:29:53         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVER_SHEAR_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVER_SHEAR_MOVER_H_

# include "PoseMover.h"

namespace lbcpp
{

class ShearMover;
typedef ReferenceCountedObjectPtr<ShearMover> ShearMoverPtr;

class ShearMover : public PoseMover
{
public:

  ShearMover();
  ShearMover(size_t residue, double deltaPhi, double deltaPsi);

  virtual void move(core::pose::PoseOP& pose) const;
  virtual void move(PosePtr& pose) const;

  static void move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi);
  static void move(PosePtr& pose, int residue, double deltaPhi, double deltaPsi);

  void setResidueIndex(size_t index);
  size_t getResidueIndex();

  void setDeltaPhi(double delta);
  double getDeltaPhi();
  void setDeltaPsi(double delta);
  double getDeltaPsi();

  virtual bool isEqual(const PoseMoverPtr& mover) const;

  virtual PoseMoverPtr getOpposite() const;

protected:
  friend class ShearMoverClass;

  size_t residue;
  double deltaPhi;
  double deltaPsi;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SHEAR_MOVER_H_
