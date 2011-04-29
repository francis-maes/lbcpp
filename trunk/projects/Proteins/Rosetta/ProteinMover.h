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

# undef T
#  include <core/pose/Pose.hh>
# define T JUCE_T

namespace lbcpp
{

class ProteinMover;
typedef ReferenceCountedObjectPtr<ProteinMover> ProteinMoverPtr;

class ProteinMover: public Object
{
public:
  ProteinMover() :
    Object(), moverName(T("Default name"))
  {
  }

  ProteinMover(const String& name) :
    Object(), moverName(name)
  {
  }

  ~ProteinMover()
  {
  }

  /**
   * Performs the perturbation on the object. Must be reimplemented in the inheriting
   * classes.
   * @param pose the pose to perturb.
   */
  virtual void move(core::pose::PoseOP& pose)=0;

  String getName()
  {
    return moverName;
  }

protected:
  friend class ProteinMoverClass;
  String moverName;
};

class RigidBodyMover;
typedef ReferenceCountedObjectPtr<RigidBodyMover> RigidBodyMoverPtr;

class RigidBodyMover: public ProteinMover
{
public:
  RigidBodyMover() :
    ProteinMover(T("Rigid body mover"))
  {
  }

  RigidBodyMover(String name):
    ProteinMover(name)
  {
  }

  /**
   * A mover object that is the parent class for other rigid body movers. Its purpose is
   * to store and manage common features of rigid body movers such as the residues
   * involved in the movement. This residues are given by the vec argument.
   *
   * @param name the name of the mover.
   * @param indexResidueOne the index of the first residue (starting from 0)
   * @param indexResidueTwo the index of the second residue (starting from 0)
   *
   * Note: The two residues must be separated by at least one other residue,
   * i.e. abs(index1-index2) >= 2.
   */
  RigidBodyMover(String name, size_t indexResidueOne, size_t indexResidueTwo) :
    ProteinMover(name), indexResidueOne(indexResidueOne), indexResidueTwo(indexResidueTwo)
  {
  }

  /**
   * Sets the new first residue to modify.
   * @param index the new index of the first residue to modify.
   */
  void setIndexResidueOne(size_t index)
  {
    indexResidueOne = index;
  }

  /**
   * Gets the current index of the first residue to modify.
   * @return the index of the current first residue.
   */
  size_t getIndexResidueOne()
  {
    return indexResidueOne;
  }

  /**
   * Sets the new second residue to modify.
   * @param index the new index of the second residue to modify.
   */
  void setIndexResidueTwo(size_t index)
  {
    indexResidueTwo = index;
  }

  /**
   * Gets the current index of the second residue to modify.
   * @return the index of the current second residue.
   */
  int getIndexResidueTwo()
  {
    return indexResidueTwo;
  }

protected:
  friend class RigidBodyMoverClass;
  size_t indexResidueOne;
  size_t indexResidueTwo;
};

extern ProteinMoverPtr phiPsiMover();
extern ProteinMoverPtr shearMover();
extern ProteinMoverPtr rigidBodyTransMover();
extern ProteinMoverPtr rigidBodySpinMover();
extern ProteinMoverPtr rigidBodyGeneralMover();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_H_
