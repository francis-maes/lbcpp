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

extern ProteinMoverPtr phiPsiMover();
extern ProteinMoverPtr shearMover();
extern ProteinMoverPtr rigidBodyGeneralMover();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MOVER_H_
