/*-----------------------------------------.---------------------------------.
| Filename:  RosettaMover.cpp              | RosettaMover                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "RosettaMover.h"

using namespace lbcpp;

// RosettaMover
RosettaMover::RosettaMover(const String& newName, long long seedForRandom) :
  NameableObject(newName)
{
  randomGenerator = new RandomGenerator(seedForRandom);
}

RosettaMover::~RosettaMover()
{
}

double RosettaMover::generateAngleFromUniform(double std, double mean)
{
  return ((std * (2 * randomGenerator->sampleDouble() - 1)) + mean);
}

double RosettaMover::generateAngleFromGaussian(double std, double mean, double limit)
{
  double ang = (std * randomGenerator->sampleDoubleFromGaussian()) + mean;
  double l = std::abs(limit);
  ang = juce::jlimit(-l, l, ang);

  return ang;
}

std::vector<String> RosettaMover::getParametersNames()
{
  std::vector<String> tmp(parameterNames);
  return tmp;
}

std::vector<Variable> RosettaMover::getParameters()
{
  updateParameters();
  std::vector<Variable> tmp(parameterValues);
  return tmp;
}

// PhiPsiRandomMover
PhiPsiRandomMover::PhiPsiRandomMover(const std::vector<Variable>& vec, long long seedForRandom) :
  RosettaMover(T("PhiPsiRandomMover"), seedForRandom)
{
  if (vec.size() >= 0)
    maxAngle = (vec.at(0)).getDouble();
  else
    maxAngle = 25.0;

  parameterNames.push_back(T("Maximum angle"));
  parameterValues.push_back(Variable((double)maxAngle));
}

void PhiPsiRandomMover::move(core::pose::PoseOP& pose)
{
  int k = randomGenerator->sampleInt(1, pose->n_residue() + 1);
  pose->set_phi(k, pose->phi(k) + generateAngleFromUniform(maxAngle));
  pose->set_psi(k, pose->psi(k) + generateAngleFromUniform(maxAngle));
}

void PhiPsiRandomMover::setMaximumAngle(double newMaxAngle)
{
  maxAngle = newMaxAngle;
}

double PhiPsiRandomMover::getMaximumAngle()
{
  return maxAngle;
}

void PhiPsiRandomMover::updateParameters()
{
  parameterValues.at(0) = Variable((double)maxAngle);
}

// PhiPsiGaussRandomMover
PhiPsiGaussRandomMover::PhiPsiGaussRandomMover(const std::vector<Variable>& vec,
    long long seedForRandom) :
  RosettaMover(T("PhiPsiGaussRandomMover"), seedForRandom)
{
  if (vec.size() >= 0)
    stdAngle = (vec.at(0)).getDouble();
  else
    stdAngle = 30;

  parameterNames.push_back(T("Standard deviation"));
  parameterValues.push_back(Variable((double)stdAngle));

}

void PhiPsiGaussRandomMover::move(core::pose::PoseOP& pose)
{
  int k = randomGenerator->sampleInt(1, pose->n_residue() + 1);
  pose->set_phi(k, pose->phi(k) + generateAngleFromGaussian(stdAngle));
  pose->set_psi(k, pose->psi(k) + generateAngleFromGaussian(stdAngle));
}

void PhiPsiGaussRandomMover::setAngleStandardDeviation(double newAngleDeviation)
{
  stdAngle = newAngleDeviation;
}

double PhiPsiGaussRandomMover::getAngleStandardDeviation()
{
  return stdAngle;
}

void PhiPsiGaussRandomMover::updateParameters()
{
  parameterValues.at(0) = Variable((double)stdAngle);
}

// ShearRandomMover
ShearRandomMover::ShearRandomMover(const std::vector<Variable>& vec, long long seedForRandom) :
  RosettaMover(T("ShearRandomMover"), seedForRandom)
{
  // Recover arguments
  if (vec.size() >= 2)
  {
    temperature = (vec.at(0)).getDouble();
    anglemax = (vec.at(1)).getDouble();
  }
  else
  {
    temperature = 2.0;
    anglemax = 10;
  }

  // Initially allow all moves
  moveMap = new core::kinematics::MoveMap;
  moveMap->set_bb(true);

  // Forbid only moves specified by the user
  for (int i = 2; i < vec.size(); i++)
    moveMap->set_bb((vec.at(i)).getInteger(), false);

  // Create mover
  mover = new protocols::moves::ShearMover(moveMap, temperature, 1);
  mover->set_angles(anglemax);

  parameterNames.push_back(T("Temperature"));
  parameterNames.push_back(T("Angle max"));
  parameterValues.push_back(Variable((double)temperature));
  parameterValues.push_back(Variable((double)anglemax));
}

void ShearRandomMover::move(core::pose::PoseOP& pose)
{
  mover->apply((*pose));
}

void ShearRandomMover::setTemperatureFactor(double newKT)
{
  temperature = newKT;
}

double ShearRandomMover::getTemperatureFactor()
{
  return temperature;
}

void ShearRandomMover::setMaximumAngle(double newMaxAngle)
{
  anglemax = newMaxAngle;
}

double ShearRandomMover::getMaximumAngle()
{
  return anglemax;
}

void ShearRandomMover::updateParameters()
{
  parameterValues[0] = Variable((double)temperature);
  parameterValues[1] = Variable((double)anglemax);
}

// RigidBodyTransRandomMover
RigidBodyTransRandomMover::RigidBodyTransRandomMover(const std::vector<Variable>& vec,
    long long seedForRandom) :
  RosettaMover(T("RigidBodyTransRandomMover"), seedForRandom)
{
  step = 0.1;
  if (vec.size() >= 0)
    step = (vec.at(0)).getDouble();

  parameterNames.push_back(T("Step"));
  parameterValues.push_back(Variable((double)step));
}

void RigidBodyTransRandomMover::move(core::pose::PoseOP& pose)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  double randomStep = randomGenerator->sampleDouble(-step, step);

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int numberResidue1 = randomGenerator->sampleInt(1, numberResidues + 1);
  int numberResidue2 = randomGenerator->sampleInt(1, numberResidues + 1);
  if (numberResidue1 == numberResidue2)
  {
    numberResidue1 = juce::jlimit((int)1, numberResidues, (int)(numberResidue1 - 1));
    numberResidue2 = juce::jlimit((int)1, numberResidues, (int)(numberResidue2 + 1));
  }
  int cutpointResidue = std::floor(((double)numberResidue1 + (double)numberResidue2) / 2.0);

  foldTree.new_jump(numberResidue1, numberResidue2, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  protocols::moves::RigidBodyTransMoverOP mover = new protocols::moves::RigidBodyTransMover(
      (*pose), 1);
  mover->step_size(randomStep);
  mover->apply((*pose));

  // Clear the jump
  foldTree.delete_jump_and_intervening_cutpoint(1);
  pose->fold_tree(foldTree);
}

void RigidBodyTransRandomMover::setMaximumDistance(double newDistance)
{
  step = newDistance;
}

double RigidBodyTransRandomMover::getMaximumDistance()
{
  return step;
}

void RigidBodyTransRandomMover::updateParameters()
{
  parameterValues[0] = Variable((double)step);
}

// RigidBodyPerturbRandomMover
RigidBodyPerturbRandomMover::RigidBodyPerturbRandomMover(const std::vector<Variable>& vec,
    long long seedForRandom) :
  RosettaMover(T("RigidBodyPerturbRandomMover"), seedForRandom)
{
  maximumStep = 0.1;
  maximumAngle = 1;
  if (vec.size() >= 2)
  {
    maximumStep = (vec.at(0)).getDouble();
    maximumAngle = (vec.at(1)).getDouble();
  }
  mover = new protocols::moves::RigidBodyPerturbMover(1, maximumAngle, maximumStep);

  parameterNames.push_back(T("Step"));
  parameterNames.push_back(T("Angle max"));
  parameterValues.push_back(Variable((double)maximumStep));
  parameterValues.push_back(Variable((double)maximumAngle));
}

void RigidBodyPerturbRandomMover::move(core::pose::PoseOP& pose)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int numberResidue1 = randomGenerator->sampleInt(1, numberResidues + 1);
  int numberResidue2 = randomGenerator->sampleInt(1, numberResidues + 1);
  if (numberResidue1 == numberResidue2)
  {
    numberResidue1 = juce::jlimit((int)1, numberResidues, (int)(numberResidue1 - 1));
    numberResidue2 = juce::jlimit((int)1, numberResidues, (int)(numberResidue2 + 1));
  }
  int cutpointResidue = std::floor(((double)numberResidue1 + (double)numberResidue2) / 2.0);

  foldTree.new_jump(numberResidue1, numberResidue2, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  mover->apply((*pose));

  // Clear the jump
  foldTree.delete_jump_and_intervening_cutpoint(1);
  pose->fold_tree(foldTree);
}

void RigidBodyPerturbRandomMover::setStandardDeviationForDistance(double newDistance)
{
  maximumStep = newDistance;
}

double RigidBodyPerturbRandomMover::getStandardDeviationForDistance()
{
  return maximumStep;
}

void RigidBodyPerturbRandomMover::setStandardDeviationForAngle(double newAngle)
{
  maximumAngle = newAngle;
}

double RigidBodyPerturbRandomMover::getStandardDeviationForAngle()
{
  return maximumAngle;
}

void RigidBodyPerturbRandomMover::updateParameters()
{
  parameterValues[0] = Variable((double)maximumStep);
  parameterValues[1] = Variable((double)maximumAngle);
}

// PhiPsiMover
PhiPsiMover::PhiPsiMover(const std::vector<Variable>& vec) :
  RosettaMover(T("PhiPsiMover"))
{
  if (vec.size() == 3)
  {
    residueNumber = (vec.at(0)).getInteger();
    phiAngleIncrement = (vec.at(1)).getDouble();
    psiAngleIncrement = (vec.at(2)).getDouble();
  }
  else
  {
    residueNumber = 1;
    phiAngleIncrement = 5;
    psiAngleIncrement = 5;
  }

  parameterNames.push_back(T("Residue Number"));
  parameterNames.push_back(T("Phi angle increment"));
  parameterNames.push_back(T("Psi angle increment"));

  parameterValues.push_back(Variable(residueNumber));
  parameterValues.push_back(Variable(phiAngleIncrement));
  parameterValues.push_back(Variable(psiAngleIncrement));
}

void PhiPsiMover::move(core::pose::PoseOP& pose)
{
  move(pose, residueNumber, phiAngleIncrement, psiAngleIncrement);
}

void PhiPsiMover::move(core::pose::PoseOP& pose, int residue, double phiAngle, double psiAngle)
{
  pose->set_phi(residue, pose->phi(residue) + phiAngle);
  pose->set_psi(residue, pose->psi(residue) + psiAngle);
}

void PhiPsiMover::setResidueNumber(int newResidueNumber)
{
  residueNumber = newResidueNumber;
}

int PhiPsiMover::getResidueNumber()
{
  return residueNumber;
}

void PhiPsiMover::setPhiIncrement(double increment)
{
  phiAngleIncrement = increment;
}

double PhiPsiMover::getPhiIncrement()
{
  return phiAngleIncrement;
}

void PhiPsiMover::setPsiIncrement(double increment)
{
  psiAngleIncrement = increment;
}

double PhiPsiMover::getPsiIncrement()
{
  return psiAngleIncrement;
}

void PhiPsiMover::updateParameters()
{
  parameterValues[0] = Variable(residueNumber);
  parameterValues[1] = Variable(phiAngleIncrement);
  parameterValues[2] = Variable(psiAngleIncrement);
}

// RigidBodyMover
RigidBodyMover::RigidBodyMover(String name, const std::vector<Variable>& vec) :
  RosettaMover(name)
{
  if (vec.size() >= 2)
  {
    firstResidueNumber = vec[0].getInteger();
    secondResidueNumber = vec[1].getInteger();
  }
  else
  {
    firstResidueNumber = 1;
    secondResidueNumber = 3;
  }
  parameterNames.push_back(T("First residue"));
  parameterNames.push_back(T("Second residue"));
  parameterValues.push_back(Variable((int)firstResidueNumber));
  parameterValues.push_back(Variable((int)secondResidueNumber));
}

void RigidBodyMover::setFirstResidueNumber(int newResidueNumber)
{
  firstResidueNumber = newResidueNumber;
}

int RigidBodyMover::getFirstResidueNumber()
{
  return firstResidueNumber;
}

void RigidBodyMover::setSecondResidueNumber(int newResidueNumber)
{
  secondResidueNumber = newResidueNumber;
}

int RigidBodyMover::getSecondResidueNumber()
{
  return secondResidueNumber;
}

// RigidBodyTransMover
RigidBodyTransMover::RigidBodyTransMover(const std::vector<Variable>& vec) :
  RigidBodyMover(T("RigidBodyTransMover"), vec)
{
  if (vec.size() == 3)
    step = vec[2].getDouble();
  else
    step = 1.0;

  parameterNames.push_back(T("Amplitude of translation"));
  parameterValues.push_back(Variable((double)step));
}

void RigidBodyTransMover::move(core::pose::PoseOP& pose)
{
  move(pose, firstResidueNumber, secondResidueNumber, step);
}

void RigidBodyTransMover::move(core::pose::PoseOP& pose, int firstResidue, int secondResidue,
    double amplitude)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  protocols::moves::RigidBodyTransMoverOP mover = new protocols::moves::RigidBodyTransMover(
      (*pose), 1);
  mover->step_size(amplitude);
  mover->apply((*pose));

  // Clear the jump
  foldTree.delete_jump_and_intervening_cutpoint(1);
  pose->fold_tree(foldTree);
}

void RigidBodyTransMover::updateParameters()
{
  parameterValues[0] = Variable((int)firstResidueNumber);
  parameterValues[1] = Variable((int)secondResidueNumber);
  parameterValues[2] = Variable((double)step);
}

void RigidBodyTransMover::setStep(double amplitude)
{
  step = amplitude;
}

double RigidBodyTransMover::getStep()
{
  return step;
}

// RigidBodySpinMover
RigidBodySpinMover::RigidBodySpinMover(const std::vector<Variable>& vec) :
  RigidBodyMover(T("RigidBodySpinMover"), vec)
{
  if (vec.size() == 3)
    extent = vec[2].getDouble();
  else
    extent = 5;

  parameterNames.push_back(T("Magnitude"));
  parameterValues.push_back(Variable(extent));
}

void RigidBodySpinMover::move(core::pose::PoseOP& pose)
{
  move(pose, firstResidueNumber, secondResidueNumber, extent);
}

void RigidBodySpinMover::move(core::pose::PoseOP& pose, int firstResidue, int secondResidue,
    double amplitude)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int cutpointResidue = std::floor(((double)firstResidue + (double)secondResidue) / 2.0);

  foldTree.new_jump(firstResidue, secondResidue, cutpointResidue);
  pose->fold_tree(foldTree);

  // Perturb the pose
  core::kinematics::Jump jumpToModify = pose->jump(1);
  core::kinematics::Stub firstStub = (pose->conformation()).upstream_jump_stub(1);
  core::kinematics::Stub secondStub = (pose->conformation()).downstream_jump_stub(1);

  // Create rotation axis and rotation center
  core::Vector upCentroids;
  core::Vector rotationCenter;
  protocols::geometry::centroids_by_jump((*pose), 1, upCentroids, rotationCenter);
  core::Vector rotationAxis = upCentroids - rotationCenter;

  // Apply rotation
  jumpToModify.set_rb_center(1, secondStub, rotationCenter);
  jumpToModify.rotation_by_axis(firstStub, rotationAxis, rotationCenter, amplitude);

  // Set new conformation and clear the jump
  pose->set_jump(1, jumpToModify);
  foldTree = pose->fold_tree();
  foldTree.delete_jump_and_intervening_cutpoint(1);
  pose->fold_tree(foldTree);
}

void RigidBodySpinMover::setMagnitude(double amplitude)
{
  extent = amplitude;
}

double RigidBodySpinMover::getMagnitude()
{
  return extent;
}

void RigidBodySpinMover::updateParameters()
{
  parameterValues[0] = Variable((int)firstResidueNumber);
  parameterValues[1] = Variable((int)secondResidueNumber);
  parameterValues[2] = Variable((double)extent);
}

RigidBodyGeneralMover::RigidBodyGeneralMover(const std::vector<Variable>& vec) :
  RigidBodyMover(T("RigidBodyGeneralMover"), vec)
{
  if (vec.size() >= 4)
  {
    magnitude = vec[2].getDouble();
    step = vec[3].getDouble();
  }
  else
  {
    magnitude = 5.0;
    step = 1.0;
  }

  parameterNames.push_back(T("Rotation magnitude"));
  parameterNames.push_back(T("Translation step"));
  parameterValues.push_back(Variable((double)magnitude));
  parameterValues.push_back(Variable((double)step));
}

void RigidBodyGeneralMover::move(core::pose::PoseOP& pose)
{
  move(pose, firstResidueNumber, secondResidueNumber, magnitude, step);
}

void RigidBodyGeneralMover::move(core::pose::PoseOP& pose, int firstResidue, int secondResidue,
    double amplitudeRotation, double stepTranslation)
{
  RigidBodySpinMover::move(pose, firstResidue, secondResidue, amplitudeRotation);
  RigidBodyTransMover::move(pose, firstResidue, secondResidue, stepTranslation);
}

void RigidBodyGeneralMover::setStep(double step)
{
  this->step = step;
}

double RigidBodyGeneralMover::getStep()
{
  return step;
}

void RigidBodyGeneralMover::setMagnitude(double magnitude)
{
  this->magnitude = magnitude;
}

double RigidBodyGeneralMover::getMagnitude()
{
  return magnitude;
}

void RigidBodyGeneralMover::updateParameters()
{
  parameterValues[0] = Variable((int)firstResidueNumber);
  parameterValues[1] = Variable((int)secondResidueNumber);
  parameterValues[2] = Variable((double)magnitude);
  parameterValues[3] = Variable((double)step);
}
