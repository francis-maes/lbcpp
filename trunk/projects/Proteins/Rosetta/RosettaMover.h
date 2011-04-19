/*-----------------------------------------.---------------------------------.
| Filename:  RosettaMover.h                | RosettaMover                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_MOVER_H_
# define LBCPP_PROTEINS_ROSETTA_MOVER_H_

# include "precompiled.h"
# include "RosettaUtils.h"

# include <cmath>
# include <vector>
# include <algorithm>

# undef T
#  include <core/kinematics/MoveMap.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <protocols/init.hh>
#  include <protocols/moves/BackboneMover.hh>
#  include <protocols/moves/Mover.hh>
#  include <protocols/moves/MoverCreator.hh>
#  include <protocols/moves/MoverFactory.hh>
#  include <protocols/moves/RigidBodyMover.hh>
# define T JUCE_T

namespace lbcpp
{
class RosettaMover;
typedef ReferenceCountedObjectPtr<RosettaMover> RosettaMoverPtr;
class RosettaMover: public NameableObject
{
public:
  /**
   * Generates a random angle drawn form a uniform distribution between
   * -std and std with an offset given by mean.
   * @param std the intervals of the distribution
   * @param mean the offset of the distribution
   * @return an angle in [mean-std; mean+std].
   */
  double generateAngleFromUniform(double std, double mean = 0);

  /**
   * Generates a random angle drawn form a gaussian distribution with
   * standard deviation given by std and mean given by mean. The angle
   * lies in the range [-limit, limit] to ensure coherence.
   * @param std the standard deviation of the distribution;
   * @param mean the mean of the distribution.
   * @param limit the bounds of the return value.
   * @return an angle in [-limit; limit] drawn from a gaussaian distribution.
   */
  double generateAngleFromGaussian(double std, double mean = 0, double limit = 180);

  /**
   * Performs the perturbation on the object. Must be reimplemented in the inheriting
   * classes.
   * @param pose the pose to perturb.
   */
  virtual void move(core::pose::PoseOP pose)=0;

  /**
   * Tells the name of the current mover.
   * @return the name of the mover.
   */
  virtual String getName();

  /**
   * Gives the names of the parameter. This is used in the trace callbacks.
   * @return a vector containing the names of the parameters.
   */
  virtual std::vector<String> getParametersNames();

  /**
   * Gives the values of the parameters. Used in trace callbacks.
   * @return a vector containing the values of the parameters. A certain
   * position in the vector corresponds to the parameter whose name is
   * given at the same position in the result of getParametersNames().
   */
  virtual std::vector<Variable> getParameters();
};

class PhiPsiRandomMover;
typedef ReferenceCountedObjectPtr<PhiPsiRandomMover> PhiPsiRandomMoverPtr;
class PhiPsiRandomMover: public RosettaMover
{
private:
  double maxAngle;
public:
  /**
   * Increments the phi and psi angles of a randomly selected residue of the pose object.
   * The increments are randomly selected in the range [-x, x] where x is given by argument vec.
   * @param vec a vector containing 1 element of type double* that gives the range where the
   * increments of the psi and phi angles lie.
   */
  PhiPsiRandomMover(const std::vector<Variable>& vec);
  void move(core::pose::PoseOP pose);
  String getName();
  std::vector<String> getParametersNames();
  std::vector<Variable> getParameters();
};

class PhiPsiGaussRandomMover;
typedef ReferenceCountedObjectPtr<PhiPsiGaussRandomMover> PhiPsiGaussRandomMoverPtr;
class PhiPsiGaussRandomMover: public RosettaMover
{
private:
  double stdAngle;
public:
  /**
   * Increments the phi and psi angles of a randomly selected residue of the pose object.
   * The increments are randomly selected by a gaussian distribution of standard deviation = x
   * where x is given by argument vec.
   * @param vec a vector containing 1 element of type double* that gives the range where the
   * increments of the psi and phi angles lie.
   */
  PhiPsiGaussRandomMover(const std::vector<Variable>& vec);
  void move(core::pose::PoseOP pose);
  String getName();
  std::vector<String> getParametersNames();
  std::vector<Variable> getParameters();
};

class ShearRandomMover;
typedef ReferenceCountedObjectPtr<ShearRandomMover> ShearRandomMoverPtr;
class ShearRandomMover: public RosettaMover
{
private:
  double temperature;
  double anglemax;
  int numberOfMoves;
  core::kinematics::MoveMapOP moveMap;
  protocols::moves::ShearMoverOP mover;
public:
  /**
   * Applies a mover on the pose. Moves randomly ith angle phi and (i-1)th angle psi to limit
   * the changes in the global structure. Moreover, the angles are verified to lie in
   * the possible Ramachandran regions by a Metropolis criterion (using kT).
   * @param vec a pointer to a vector of variables that are : first, a double containing
   * the value of kT used for the Metropolis criterion; second, the maximum angle by
   * which the angles phi and psi can be perturbed; third, the number of perturbations
   * performed.
   */
  ShearRandomMover(const std::vector<Variable>& vec);
  void move(core::pose::PoseOP pose);
  String getName();
  std::vector<String> getParametersNames();
  std::vector<Variable> getParameters();
};

class RigidBodyTransRandomMover;
typedef ReferenceCountedObjectPtr<RigidBodyTransRandomMover> RigidBodyTransRandomMoverPtr;
class RigidBodyTransRandomMover: public RosettaMover
{
private:
  double step;
public:
  /**
   * Applies a random rigid body transformation on the object. This modifier changes the
   * distance between two residues while maintaining the rest of the conformation more or
   * less unchanged.
   * @param vec a pointer to a vector of variables that are : first, a double containing
   * the value X of the distance used to move the residues. The distance by which the
   * residues are moved is drawn from a uniform distribution in the interval [-X;X].
   */
  RigidBodyTransRandomMover(const std::vector<Variable>& vec);
  void move(core::pose::PoseOP pose);
  String getName();
  std::vector<String> getParametersNames();
  std::vector<Variable> getParameters();
};

class RigidBodyPerturbRandomMover;
typedef ReferenceCountedObjectPtr<RigidBodyPerturbRandomMover> RigidBodyPerturbRandomMoverPtr;
class RigidBodyPerturbRandomMover: public RosettaMover
{
private:
  double maximumStep;
  double maximumAngle;
  protocols::moves::RigidBodyPerturbMoverOP mover;
public:
  /**
   * Applies a random rigid body transformation on the object. This modifier changes the
   * distance between two residues and the orientation between them while maintaining
   * the rest of the conformation more or less unchanged.
   * @param vec a pointer to a vector of variables that are : first, a double containing
   * the value X of the distance used to move the residues, the distance by which the
   * residues are moved is drawn from a gaussian distribution with standard deviation X;
   * second, a double Y corresponding to the standard deviation used to draw an angle
   * from a gaussian distribution.
   */
  RigidBodyPerturbRandomMover(const std::vector<Variable>& vec);
  void move(core::pose::PoseOP pose);
  String getName();
  std::vector<String> getParametersNames();
  std::vector<Variable> getParameters();
};
}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_MOVER_H_
