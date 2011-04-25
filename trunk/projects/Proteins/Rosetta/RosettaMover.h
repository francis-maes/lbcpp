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
#  include <core/conformation/Conformation.hh>
#  include <protocols/init.hh>
#  include <protocols/moves/BackboneMover.hh>
#  include <protocols/moves/Mover.hh>
#  include <protocols/moves/MoverCreator.hh>
#  include <protocols/moves/MoverFactory.hh>
#  include <protocols/moves/RigidBodyMover.hh>
#  include <protocols/geometry/RB_geometry.hh>
#  include <core/kinematics/Jump.hh>
#  include <core/kinematics/Stub.hh>
# define T JUCE_T

namespace lbcpp
{
class RosettaMover;
typedef ReferenceCountedObjectPtr<RosettaMover> RosettaMoverPtr;
class RosettaMover: public NameableObject
{
public:
  RosettaMover(const String& newName, long long seedForRandom = 0);
  ~RosettaMover();
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
  virtual void move(core::pose::PoseOP& pose)=0;

  /**
   * Gives the names of the parameter. This is used in the trace callbacks.
   * @return a vector containing the names of the parameters.
   */
  std::vector<String> getParametersNames();

  /**
   * Gives the values of the parameters. Used in trace callbacks.
   * @return a vector containing the values of the parameters. A certain
   * position in the vector corresponds to the parameter whose name is
   * given at the same position in the result of getParametersNames().
   */
  std::vector<Variable> getParameters();

protected:
  /**
   * Methods that must be reimplemented in each subclass. This ensures the
   * refresh of the paramaterValues vector before throwing it in the
   * method getParameters. See examples for implementation.
   */
  virtual void updateParameters()=0;

  RandomGeneratorPtr randomGenerator;
  std::vector<String> parameterNames;
  std::vector<Variable> parameterValues;
};

class PhiPsiRandomMover;
typedef ReferenceCountedObjectPtr<PhiPsiRandomMover> PhiPsiRandomMoverPtr;
class PhiPsiRandomMover: public RosettaMover
{
public:
  /**
   * A mover that changes the phi and psi angles of a randomly selected residue
   * of the pose object. The increments are randomly selected in the range [-x, x]
   * where x is given by argument vec. The changes are performed when the method move
   * is called.
   *
   * @param vec a vector containing 1 element :
   * - angle, of type double that gives the range where the increments of the psi
   *   and phi angles lie.
   */
  PhiPsiRandomMover(const std::vector<Variable>& vec, long long seedForRandom = 0);

  /**
   * Applies the mover on the pose.
   * @param pose the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Sets the maximum value of the angles phi and psi.
   * @param newMaxAngle the new value for the bounds of the angles.
   */
  void setMaximumAngle(double newMaxAngle);

  /**
   * Get the value of the maximum possible angles.
   * @return the maximum angle value.
   */
  double getMaximumAngle();

protected:
  void updateParameters();

private:
  double maxAngle;
};

class PhiPsiGaussRandomMover;
typedef ReferenceCountedObjectPtr<PhiPsiGaussRandomMover> PhiPsiGaussRandomMoverPtr;
class PhiPsiGaussRandomMover: public RosettaMover
{
public:
  /**
   * A mover that changes the phi and psi angles of a randomly selected residue
   * of the pose object. The increments are randomly selected in a gaussian distribution
   * with tandard deviation given by x where x is given by argument vec. The changes
   * are performed when the method move is called.
   *
   * @param vec a vector containing 1 element:
   * - angle : of type double that gives the standard deviation of the distribution
   *   used to sample the psi and phi angles.
   */
  PhiPsiGaussRandomMover(const std::vector<Variable>& vec, long long seedForRandom = 0);

  /**
   * Applies the mover on the pose.
   * @param pose the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Sets the new standard deviation of the gaussian distribution.
   * @param newAngleDeviation the new value for standard deviation.
   */
  void setAngleStandardDeviation(double newAngleDeviation);

  /**
   * Get the current value for standard deviation.
   * @return the current value of standard deviation.
   */
  double getAngleStandardDeviation();

protected:
  void updateParameters();

private:
  double stdAngle;
};

class ShearRandomMover;
typedef ReferenceCountedObjectPtr<ShearRandomMover> ShearRandomMoverPtr;
class ShearRandomMover: public RosettaMover
{
public:
  /**
   * A mover that moves randomly ith angle phi and (i-1)th angle psi to limit
   * the changes in the global structure. Moreover, the angles are verified to lie in
   * the possible Ramachandran regions by a Metropolis criterion (using kT).
   *
   * @param vec a pointer to a vector of variables that are :
   * - temperature, a double containing the value of kT used for the Metropolis
   *   criterion;
   * - step, the maximum angle by which the angles phi and psi can be perturbed.
   * - The other variables contained in vec are integers that specify which residues
   * are not allowed to move.
   *
   * Note : Only the first two arguments are required, the others are facultative.
   */
  ShearRandomMover(const std::vector<Variable>& vec, long long seedForRandom = 0);

  /**
   * Performs the move on the pose.
   * @param pose the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Sets the new temperature factor for the Metropolis criterion used to
   * validate the angles with respect to Ramachandran plot.
   * @param newKT the new KT.
   */
  void setTemperatureFactor(double newKT);

  /**
   * Get the current temperature factor.
   * @return the current temperature factor.
   */
  double getTemperatureFactor();

  /**
   * Sets the maximum angle by which the phi and psi angles can be perturbed.
   * @param newMaxAngle the new maximum angle.
   */
  void setMaximumAngle(double newMaxAngle);

  /**
   * Get the current value for the maximum angle.
   * @return the current value for the maximum angle.
   */
  double getMaximumAngle();

protected:
  void updateParameters();

private:
  double temperature;
  double anglemax;
  core::kinematics::MoveMapOP moveMap;
  protocols::moves::ShearMoverOP mover;
};

class RigidBodyTransRandomMover;
typedef ReferenceCountedObjectPtr<RigidBodyTransRandomMover> RigidBodyTransRandomMoverPtr;
class RigidBodyTransRandomMover: public RosettaMover
{
public:
  /**
   * A mover that applies a random rigid body transformation on the object. This modifier
   * changes the distance between two residues while maintaining the rest of the
   * conformation more or less unchanged.
   *
   * @param vec a pointer to a vector of variables that are :
   * - step, a double containing the value X of the distance used to move the residues.
   *   The distance by which the residues are moved is drawn from a uniform distribution
   *   in the interval [-X;X].
   */
  RigidBodyTransRandomMover(const std::vector<Variable>& vec, long long seedForRandom = 0);

  /**
   * Applies the mover on the pose object.
   * @param pose the object to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Sets the new maximum distance by which the two residues can be moved along their axis.
   * @param newDistance the new maximum distance.
   */
  void setMaximumDistance(double newDistance);

  /**
   * Get the current maximum distance.
   * @return the current maximum distance.
   */
  double getMaximumDistance();

protected:
  void updateParameters();

private:
  double step;
};

class RigidBodyPerturbRandomMover;
typedef ReferenceCountedObjectPtr<RigidBodyPerturbRandomMover> RigidBodyPerturbRandomMoverPtr;
class RigidBodyPerturbRandomMover: public RosettaMover
{
public:
  /**
   * A mover that applies a random rigid body transformation on the object. This modifier
   * changes the distance between two residues and the orientation between them while
   * maintaining the rest of the conformation more or less unchanged.
   *
   * @param vec a pointer to a vector of variables that are :
   * - step, a double containing the value X of the distance used to move the residues,
   *   the distance by which the residues are moved is drawn from a gaussian distribution
   *   with standard deviation X;
   * - angle, a double Y corresponding to the standard deviation used to draw an angle
   *   from a gaussian distribution.
   */
  RigidBodyPerturbRandomMover(const std::vector<Variable>& vec, long long seedForRandom = 0);

  /**
   * Applies the mover on the pose object.
   * @param pose the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Sets the new standard deviation used to draw a distance to move the residues.
   * @param newDistance the new standard deviation.
   */
  void setStandardDeviationForDistance(double newDistance);

  /**
   * Get the current standard deviation for the distance.
   * @return the current standard deviation for the distance.
   */
  double getStandardDeviationForDistance();

  /**
   * Sets the new standard deviation used to draw an angle to move the residues.
   * @param newAngle the new standard deviation.
   */
  void setStandardDeviationForAngle(double newAngle);

  /**
   * Get the current standard deviation for the distance.
   * @return the current standard deviation for the distance.
   */
  double getStandardDeviationForAngle();

protected:
  void updateParameters();

private:
  double maximumStep;
  double maximumAngle;
  protocols::moves::RigidBodyPerturbMoverOP mover;
};

class PhiPsiMover;
typedef ReferenceCountedObjectPtr<PhiPsiMover> PhiPsiMoverPtr;
class PhiPsiMover: public RosettaMover
{
public:
  /**
   * Instantiates a mover object that performs a modification on the specified residue.
   * The residue and the values of the increments of the Phi and Psi angles are given by
   * the vec argument.
   *
   * @param vec a vector containing the arguments:
   * - residue1, is an integer in [1, number residues] that specifies the residue
   *   to modify,
   * - anglePhi, is a double specifying the value of the increment of the PHI angle,
   * - anglePsi, is a double specifying a value for the PSI angle increment.
   */
  PhiPsiMover(const std::vector<Variable>& vec);

  /**
   * Performs the move on the pose specified by the parameters of the mover.
   * @param pose the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Moves the angles Phi and Psi of the residue specified by an amount specified.
   *
   * @param pose the pose object to modify.
   * @param residue the residue whose angles are to be modified.
   * @parm phiAngle the amount by which the Phi angle of the specified residue has
   * to be modified.
   * @parm psiAngle the amount by which the Psi angle of the specified residue has
   * to be modified.
   */
  static void move(core::pose::PoseOP& pose, int residue, double phiAngle, double psiAngle);

  /**
   * Sets the new residue to modify.
   * @param newResidueNumber the new index of the residue to modify.
   */
  void setResidueNumber(int newResidueNumber);

  /**
   * Gets the current index of the residue to modify.
   * @return the index of the current residue.
   */
  int getResidueNumber();

  /**
   * Sets the new increment for the PHI angle.
   * @param increment the new value for the PHI increment.
   */
  void setPhiIncrement(double increment);

  /**
   * Gets the current value for the PHI increment.
   * @return the current PHI increment.
   */
  double getPhiIncrement();

  /**
   * Sets the new increment for the PSI angle.
   * @param increment the new value for the PSI increment.
   */
  void setPsiIncrement(double increment);

  /**
   * Gets the current value for the PSI increment.
   * @return the current PSI increment.
   */
  double getPsiIncrement();

protected:
  void updateParameters();

private:
  int residueNumber;
  double phiAngleIncrement;
  double psiAngleIncrement;
};

class RigidBodyMover;
typedef ReferenceCountedObjectPtr<RigidBodyMover> RigidBodyMoverPtr;
class RigidBodyMover: public RosettaMover
{
public:
  /**
   * A mover object that is the parent class for other rigid body movers. Its purpose is
   * to store and manage common features of rigid body movers such as the residues
   * involved in the movement. This residues are given by the vec argument.
   *
   * @param name the name of the mover.
   * @param vec a vector containing the arguments:
   * - residue1, is an integer in [1, number residues] that specifies the first residue
   *   to modify,
   * - residue2, is an integer specifying the second residue in [1, number residues].
   *
   * Note: The two residues must be separated by at least an other residue,
   * i.e. abs(index1-index2) >= 2.
   */
  RigidBodyMover(String name, const std::vector<Variable>& vec);

  /**
   * Sets the new first residue to modify.
   * @param newResidueNumber the new index of the first residue to modify.
   */
  void setFirstResidueNumber(int newResidueNumber);

  /**
   * Gets the current index of the first residue to modify.
   * @return the index of the current first residue.
   */
  int getFirstResidueNumber();

  /**
   * Sets the new second residue to modify.
   * @param newResidueNumber the new index of the second residue to modify.
   */
  void setSecondResidueNumber(int newResidueNumber);

  /**
   * Gets the current index of the second residue to modify.
   * @return the index of the current second residue.
   */
  int getSecondResidueNumber();

protected:
  int firstResidueNumber;
  int secondResidueNumber;
};

class RigidBodyTransMover;
typedef ReferenceCountedObjectPtr<RigidBodyTransMover> RigidBodyTransMoverPtr;
class RigidBodyTransMover: public RigidBodyMover
{
public:
  /**
   * Instantiates a mover object that performs a translation of two residues following
   * the axis formed by them. The residues and the amplitude of the translation are
   * given by the vec argument.
   *
   * @param vec a vector containing the arguments:
   * - residue1, is an integer in [1, number residues] that specifies the first residue
   *   to modify,
   * - residue2, is an integer specifying the second residue in [1, number residues],
   * - step, is a double specifying the step of the translation.
   *
   * Note: The two residues must be separated by at least an other residue,
   * i.e. abs(index1-index2) >= 2.
   */
  RigidBodyTransMover(const std::vector<Variable>& vec);

  /**
   * Performs the translation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Moves the two residues by a translation following the axis that is formed
   * by the two specified residues.
   * @param pose the pose object to modify.
   * @param firstResidue the first residue.
   * @param secondResidue the second residue.
   * @parm step the amplitude of the translation.
   */
  static void move(core::pose::PoseOP& pose, int firstResidue, int secondResidue, double amplitude);

  /**
   * Sets the new increment for the distance.
   * @param amplitude the new value for the distance increment.
   */
  void setStep(double amplitude);

  /**
   * Gets the current value for the distance increment.
   * @return the current distance increment.
   */
  double getStep();

protected:
  void updateParameters();

private:
  double step;
};

class RigidBodySpinMover;
typedef ReferenceCountedObjectPtr<RigidBodySpinMover> RigidBodySpinMoverPtr;
class RigidBodySpinMover: public RigidBodyMover
{
public:
  /**
   * Instantiates a mover object that performs a rotation around the axis formed by the
   * two specified residues. The residues and the magnitude of the rotation are
   * given by the vec argument.
   *
   * @param vec a vector containing the arguments:
   * - residue1, is an integer in [1, number residues] that specifies the first
   *   residue to modify,
   * - residue2, is an integer specifying the second residue in [1, number residues],
   * - magnitude, is a double specifying the magnitude of the rotation.
   *
   * Note: The two residues must be separated by at least an other residue, i.e.
   * abs(index1-index2) >= 2.
   */
  RigidBodySpinMover(const std::vector<Variable>& vec);

  /**
   * Performs the rotation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Moves the two residues by a rotation around the axis that is formed
   * by the two specified residues.
   * @param pose the pose object to modify.
   * @param firstResidue the first residue.
   * @param secondResidue the second residue.
   * @parm amplitude the magnitude of the rotation.
   */
  static void move(core::pose::PoseOP& pose, int firstResidue, int secondResidue, double amplitude);

  /**
   * Sets the new rotation angle.
   * @param apmplitude the new rotation angle.
   */
  void setMagnitude(double amplitude);

  /**
   * Gets the current value for the rotation angle.
   * @return the current value for the rotation angle.
   */
  double getMagnitude();

protected:
  void updateParameters();

private:
  double extent;
};

class RigidBodyGeneralMover;
typedef ReferenceCountedObjectPtr<RigidBodyGeneralMover> RigidBodyGeneralMoverPtr;
class RigidBodyGeneralMover: public RigidBodyMover
{
public:
  /**
   * Instantiates a mover object that performs a rotation around the axis formed by the
   * two specified residues followed by a translation along this axis. The residues, the
   * magnitude of the rotation and the step of the translation are given by the vec argument.
   *
   * @param vec a vector containing the argument:
   * - residue1, is an integer in [1, number residues] that specifies the residue to modify,
   * - residue2, is an integer specifying the second residue in [1, number residues],
   * - magnitude, is a double specifying the magnitude of the rotation,
   * - step, is a double specifying the step of the translation.
   *
   * Note: The two residues must be separated by at least an other residue, i.e.
   * abs(index1-index2) >= 2.
   */
  RigidBodyGeneralMover(const std::vector<Variable>& vec);

  /**
   * Performs the rotation on the pose specified by the parameters of the mover.
   * @param the pose to modify.
   */
  void move(core::pose::PoseOP& pose);

  /**
   * Moves the two residues by a rotation around the axis that is formed
   * by the two specified residues and a translation along the same axis.
   * @param pose the pose object to modify.
   * @param firstResidue the first residue.
   * @param secondResidue the second residue.
   * @param amplitudeRotation the magnitude of the rotation.
   * @param stepTranslation the distance by which the residues are moved.
   */
  static void move(core::pose::PoseOP& pose, int firstResidue, int secondResidue,
      double amplitudeRotation, double stepTranslation);

  /**
   * Sets the new increment for the distance.
   * @param amplitude the new value for the distance increment.
   */
  void setStep(double step);

  /**
   * Gets the current value for the distance increment.
   * @return the current distance increment.
   */
  double getStep();

  /**
   * Sets the new rotation angle.
   * @param apmplitude the new rotation angle.
   */
  void setMagnitude(double magnitude);

  /**
   * Gets the current value for the rotation angle.
   * @return the current value for the rotation angle.
   */
  double getMagnitude();

protected:
  void updateParameters();

private:
  double step;
  double magnitude;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_MOVER_H_
