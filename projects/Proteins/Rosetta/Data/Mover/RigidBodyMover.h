/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyMover.h               | RigidBodyMover                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 mai 2011  18:08:06          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVER_RIGIDBODYMOVER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVER_RIGIDBODYMOVER_H_

# include "PoseMover.h"

namespace lbcpp
{

class RigidBodyMover;
typedef ReferenceCountedObjectPtr<RigidBodyMover> RigidBodyMoverPtr;

class RigidBodyMover : public PoseMover
{
public:
  RigidBodyMover();
  RigidBodyMover(size_t indexResidueOne, size_t indexResidueTwo, double magnitude, double amplitude);
  explicit RigidBodyMover(const RigidBodyMover& mover);

  virtual void move(core::pose::PoseOP& pose) const;
  virtual void move(PosePtr& pose) const;
  static void move(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitudeTranslation,
      double amplitudeRotation);
  static void move(PosePtr& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitudeTranslation,
      double amplitudeRotation);

  void setAmplitude(double newAmplitude);
  void setMagnitude(double newMagnitude);
  double getAmplitude() const;
  double getMagnitude() const;

  void setIndexResidueOne(size_t index);
  void setIndexResidueTwo(size_t index);
  size_t getIndexResidueOne() const;
  size_t getIndexResidueTwo() const;

  virtual bool isEqual(const PoseMoverPtr& mover) const;

  virtual PoseMoverPtr getOpposite() const;

protected:
  static void applyTranslation(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double magnitude);
  static void applyRotation(core::pose::PoseOP& pose, size_t indexResidueOne, size_t indexResidueTwo, double amplitude);

  friend class RigidBodyMoverClass;

  PairPtr residues;
  double magnitude; // for translation
  double amplitude; // for rotation
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVER_RIGIDBODYMOVER_H_
