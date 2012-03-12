/*-----------------------------------------.---------------------------------.
| Filename: PhiPsiMover.h                  | PhiPsiMover                     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  16:28:24         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVER_PHI_PSI_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVER_PHI_PSI_MOVER_H_

# include "PoseMover.h"

namespace lbcpp
{

class PhiPsiMover;
typedef ReferenceCountedObjectPtr<PhiPsiMover> PhiPsiMoverPtr;

class PhiPsiMover : public PoseMover
{
public:
  PhiPsiMover();
  PhiPsiMover(size_t residue, double deltaPhi, double deltaPsi);

  virtual void move(core::pose::PoseOP& pose) const;
  virtual void move(PosePtr& pose) const;
  static void move(core::pose::PoseOP& pose, int residue, double deltaPhi, double deltaPsi);
  static void move(PosePtr& pose, int residue, double deltaPhi, double deltaPsi);

  void setResidueIndex(size_t index);
  size_t getResidueIndex() const;

  void setDeltaPhi(double delta);
  void setDeltaPsi(double delta);
  double getDeltaPhi() const;
  double getDeltaPsi() const;

  virtual bool isEqual(const PoseMoverPtr& mover) const;
  virtual PoseMoverPtr getOpposite() const;

protected:
  friend class PhiPsiMoverClass;

  size_t residue;
  double deltaPhi;
  double deltaPsi;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_H_
