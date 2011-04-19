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
double RosettaMover::generateAngleFromUniform(double std, double mean)
{
  return ((std * (2 * generateRand() - 1)) + mean);
}

double RosettaMover::generateAngleFromGaussian(double std, double mean, double limit)
{
  double ang = (std * generateNormalRand()) + mean;
  double l = std::abs(limit);
  ang = juce::jlimit(-l, l, ang);

  return ang;
}

String RosettaMover::getName()
{
  String tmp("Default");
  return tmp;
}

std::vector<String> RosettaMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Default param name"));
  return tmp;
}

std::vector<Variable> RosettaMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable(T("Default param")));
  return tmp;
}

// PhiPsiRandomMover
PhiPsiRandomMover::PhiPsiRandomMover(const std::vector<Variable>& vec)
{
  if (vec.size() >= 0)
    maxAngle = (vec.at(0)).getDouble();
  else
    maxAngle = 25;
}

void PhiPsiRandomMover::move(core::pose::PoseOP pose)
{
  int k = ((int)rand() % pose->n_residue()) + 1;
  pose->set_phi(k, pose->phi(k) + generateAngleFromUniform(maxAngle));
  pose->set_psi(k, pose->psi(k) + generateAngleFromUniform(maxAngle));
}

String PhiPsiRandomMover::getName()
{
  return String("PhiPsiRandomMover");
}

std::vector<String> PhiPsiRandomMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Maximum angle"));
  return tmp;
}

std::vector<Variable> PhiPsiRandomMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable((double)maxAngle));
  return tmp;
}

// PhiPsiGaussRandomMover
PhiPsiGaussRandomMover::PhiPsiGaussRandomMover(const std::vector<Variable>& vec)
{
  if (vec.size() >= 0)
    stdAngle = (vec.at(0)).getDouble();
  else
    stdAngle = 30;
}

void PhiPsiGaussRandomMover::move(core::pose::PoseOP pose)
{
  int k = ((int)rand() % pose->n_residue()) + 1;
  pose->set_phi(k, pose->phi(k) + generateAngleFromGaussian(stdAngle));
  pose->set_psi(k, pose->psi(k) + generateAngleFromGaussian(stdAngle));
}

String PhiPsiGaussRandomMover::getName()
{
  return String("PhiPsiGaussRandomMover");
}

std::vector<String> PhiPsiGaussRandomMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Standard deviation"));
  return tmp;
}

std::vector<Variable> PhiPsiGaussRandomMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable((double)stdAngle));
  return tmp;
}

// ShearRandomMover
ShearRandomMover::ShearRandomMover(const std::vector<Variable>& vec)
{
  // Recover arguments
  if (vec.size() >= 3)
  {
    temperature = (vec.at(0)).getDouble();
    anglemax = (vec.at(1)).getDouble();
    numberOfMoves = (vec.at(2)).getInteger();
  }
  else
  {
    temperature = 2.0;
    anglemax = 10;
    numberOfMoves = 1;
  }

  // Initially allow all moves
  moveMap = new core::kinematics::MoveMap;
  moveMap->set_bb(true);

  // Forbid only moves specified by the user
  for (int i = 3; i < vec.size(); i++)
    moveMap->set_bb((vec.at(i)).getInteger(), false);

  // Create mover and apply to pose
  mover = new protocols::moves::ShearMover(moveMap, temperature, numberOfMoves);
  mover->set_angles(anglemax);
}

void ShearRandomMover::move(core::pose::PoseOP pose)
{
  mover->apply((*pose));
}

String ShearRandomMover::getName()
{
  return String("ShearRandomMover");
}

std::vector<String> ShearRandomMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Temperature"));
  tmp.push_back(T("Angle max"));
  tmp.push_back(T("Number of moves"));
  return tmp;
}

std::vector<Variable> ShearRandomMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable((double)temperature));
  tmp.push_back(Variable((double)anglemax));
  tmp.push_back(Variable((int)numberOfMoves));
  return tmp;
}

// RigidBodyTransRandomMover
RigidBodyTransRandomMover::RigidBodyTransRandomMover(const std::vector<Variable>& vec)
{
  step = 0.1;
  if (vec.size() >= 0)
    step = (vec.at(0)).getDouble();
}

void RigidBodyTransRandomMover::move(core::pose::PoseOP pose)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  double randomStep = step * ((2 * generateRand()) - 1);

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int numberResidue1 = std::ceil(numberResidues * generateRand());
  int numberResidue2 = std::ceil(numberResidues * generateRand());
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

String RigidBodyTransRandomMover::getName()
{
  return String("RigidBodyTransRandomMover");
}

std::vector<String> RigidBodyTransRandomMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Step"));
  return tmp;
}

std::vector<Variable> RigidBodyTransRandomMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable((double)step));
  return tmp;
}

// RigidBodyPerturbRandomMover
RigidBodyPerturbRandomMover::RigidBodyPerturbRandomMover(const std::vector<Variable>& vec)
{
  maximumStep = 0.1;
  maximumAngle = 1;
  if (vec.size() >= 2)
  {
    maximumStep = (vec.at(0)).getDouble();
    maximumAngle = (vec.at(1)).getDouble();
  }
  mover = new protocols::moves::RigidBodyPerturbMover(1, maximumAngle, maximumStep);
}

void RigidBodyPerturbRandomMover::move(core::pose::PoseOP pose)
{
  int numberResidues = (int)pose->n_residue();
  if (numberResidues < 2)
    return;

  // Set a jump
  core::kinematics::FoldTree foldTree = pose->fold_tree();
  int numberResidue1 = std::ceil(numberResidues * generateRand());
  int numberResidue2 = std::ceil(numberResidues * generateRand());
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

String RigidBodyPerturbRandomMover::getName()
{
  return String("RigidBodyPerturbRandomMover");
}

std::vector<String> RigidBodyPerturbRandomMover::getParametersNames()
{
  std::vector<String> tmp;
  tmp.push_back(T("Step"));
  tmp.push_back(T("Angle max"));
  return tmp;
}

std::vector<Variable> RigidBodyPerturbRandomMover::getParameters()
{
  std::vector<Variable> tmp;
  tmp.push_back(Variable((double)maximumStep));
  tmp.push_back(Variable((double)maximumAngle));
  return tmp;
}
