/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaMovers.cpp
 */

#include "RosettaMover.h"

namespace lbcpp
{

// RosettaMover
double RosettaMover::generateAngleFromUniform(double std, double mean)
{
	return ((std * (2 * generateRand() - 1)) + mean);
}

double RosettaMover::generateAngleFromGaussian(double std, double mean, double limit)
{
	double ang = (std * generateNormalRand()) + mean;
	double l = std::abs(limit);
	ang = std::min(std::max(-l, ang), l);

	return ang;
}

juce::String RosettaMover::getName()
{
	juce::String tmp("Default");
	return tmp;
}

std::vector<juce::String> RosettaMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Default param name"));
	return tmp;
}

std::vector<Variable> RosettaMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable(T("Default param")));
	return tmp;
}

// PhiPsiRandomMover
PhiPsiRandomMover::PhiPsiRandomMover(std::vector<Variable>* vec)
{
	if ((vec != NULL) && (vec->size() >= 0))
		step = (vec->at(0)).getDouble();
	else
		step = 25;
}

void PhiPsiRandomMover::move(core::pose::PoseOP pose)
{
	int k = ((int) rand() % pose->n_residue()) + 1;
	pose->set_phi(k, pose->phi(k) + generateAngleFromUniform(step));
	pose->set_psi(k, pose->psi(k) + generateAngleFromUniform(step));
}

juce::String PhiPsiRandomMover::getName()
{
	juce::String tmp("PhiPsiRandomMover");
	return tmp;
}

std::vector<juce::String> PhiPsiRandomMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Step"));
	return tmp;
}

std::vector<Variable> PhiPsiRandomMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable((double) step));
	return tmp;
}

// PhiPsiGaussRandomMover
PhiPsiGaussRandomMover::PhiPsiGaussRandomMover(std::vector<Variable>* vec)
{
	if ((vec != NULL) && (vec->size() >= 0))
		step = (vec->at(0)).getDouble();
	else
		step = 30;
}

void PhiPsiGaussRandomMover::move(core::pose::PoseOP pose)
{
	int k = ((int) rand() % pose->n_residue()) + 1;
	pose->set_phi(k, pose->phi(k) + generateAngleFromGaussian(step));
	pose->set_psi(k, pose->psi(k) + generateAngleFromGaussian(step));
}

juce::String PhiPsiGaussRandomMover::getName()
{
	juce::String tmp("PhiPsiGaussRandomMover");
	return tmp;
}

std::vector<juce::String> PhiPsiGaussRandomMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Standard deviation"));
	return tmp;
}

std::vector<Variable> PhiPsiGaussRandomMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable((double) step));
	return tmp;
}

// ShearRandomMover
ShearRandomMover::ShearRandomMover(std::vector<Variable>* vec)
{
	// Recover arguments
	if ((vec != NULL) && (vec->size() >= 3))
	{
		temp = (vec->at(0)).getDouble();
		anglemax = (vec->at(1)).getDouble();
		nmoves = (vec->at(2)).getInteger();
	}
	else
	{
		temp = 2.0;
		anglemax = 10;
		nmoves = 1;
	}

	// Initially allow all moves
	map = new core::kinematics::MoveMap;
	map->set_bb(true);

	// Forbid only moves specified by the user
	for (int i = 3; i < vec->size(); i++)
		map->set_bb((vec->at(i)).getInteger(), false);

	// Create mover and apply to pose
	mov = new protocols::moves::ShearMover(map, temp, nmoves);
	mov->set_angles(anglemax);
}

void ShearRandomMover::move(core::pose::PoseOP pose)
{
	mov->apply((*pose));
}

juce::String ShearRandomMover::getName()
{
	juce::String tmp("ShearRandomMover");
	return tmp;
}

std::vector<juce::String> ShearRandomMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Temperature"));
	tmp.push_back(T("Angle max"));
	tmp.push_back(T("Number of moves"));
	return tmp;
}

std::vector<Variable> ShearRandomMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable((double) temp));
	tmp.push_back(Variable((double) anglemax));
	tmp.push_back(Variable((int) nmoves));
	return tmp;
}

// RigidBodyTransRandomMover
RigidBodyTransRandomMover::RigidBodyTransRandomMover(std::vector<Variable>* vec)
{
	step = 0.1;
	if ((vec != NULL) && (vec->size() >= 0))
		step = (vec->at(0)).getDouble();
}

void RigidBodyTransRandomMover::move(core::pose::PoseOP pose)
{
	int numRes = (int) pose->n_residue();
	if (numRes < 2)
		return;

	double s = step * ((2 * generateRand()) - 1);

	// Set a jump
	core::kinematics::FoldTree ft = pose->fold_tree();
	int num1 = std::ceil(numRes * generateRand());
	int num2 = std::ceil(numRes * generateRand());
	if (num1 == num2)
	{
		num1 = std::min((int) std::max(1, num1 - 1), (int) numRes);
		num2 = std::min((int) std::max(1, num2 + 1), (int) numRes);
	}
	int cp = std::floor(((double) num1 + (double) num2) / 2.0);

	ft.new_jump(num1, num2, cp);
	pose->fold_tree(ft);

	// Perturb the pose
	protocols::moves::RigidBodyTransMoverOP mov = new protocols::moves::RigidBodyTransMover(
			(*pose), 1);
	mov->step_size(s);
	mov->apply((*pose));

	// Clear the jump
	ft.delete_jump_and_intervening_cutpoint(1);
	pose->fold_tree(ft);
}

juce::String RigidBodyTransRandomMover::getName()
{
	juce::String tmp("RigidBodyTransRandomMover");
	return tmp;
}

std::vector<juce::String> RigidBodyTransRandomMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Step"));
	return tmp;
}

std::vector<Variable> RigidBodyTransRandomMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable((double) step));
	return tmp;
}

// RigidBodyPerturbRandomMover
RigidBodyPerturbRandomMover::RigidBodyPerturbRandomMover(std::vector<Variable>* vec)
{
	step = 0.1;
	ang = 1;
	if ((vec != NULL) && (vec->size() >= 2))
	{
		step = (vec->at(0)).getDouble();
		ang = (vec->at(1)).getDouble();
	}
	mov = new protocols::moves::RigidBodyPerturbMover(1, ang, step);
}

void RigidBodyPerturbRandomMover::move(core::pose::PoseOP pose)
{
	int numRes = (int) pose->n_residue();
	if (numRes < 2)
		return;

	// Set a jump
	core::kinematics::FoldTree ft = pose->fold_tree();
	int num1 = std::ceil(numRes * generateRand());
	int num2 = std::ceil(numRes * generateRand());
	if (num1 == num2)
	{
		num1 = std::min((int) std::max(1, num1 - 1), numRes);
		num2 = std::min((int) std::max(1, num2 + 1), numRes);
	}
	int cp = std::floor(((double) num1 + (double) num2) / 2.0);

	ft.new_jump(num1, num2, cp);
	pose->fold_tree(ft);

	// Perturb the pose
	mov->apply((*pose));

	// Clear the jump
	ft.delete_jump_and_intervening_cutpoint(1);
	pose->fold_tree(ft);
}

juce::String RigidBodyPerturbRandomMover::getName()
{
	juce::String tmp("RigidBodyPerturbRandomMover");
	return tmp;
}

std::vector<juce::String> RigidBodyPerturbRandomMover::getParamNames()
{
	std::vector<juce::String> tmp;
	tmp.push_back(T("Step"));
	tmp.push_back(T("Angle max"));
	return tmp;
}

std::vector<Variable> RigidBodyPerturbRandomMover::getParams()
{
	std::vector<Variable> tmp;
	tmp.push_back(Variable((double) step));
	tmp.push_back(Variable((double) ang));
	return tmp;
}

}
; /* namespace lbcpp */
