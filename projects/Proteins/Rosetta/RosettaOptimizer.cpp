/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaOptimizer.h
 */

#include "RosettaOptimizer.h"

namespace lbcpp
{

bool keepConformation(double deltaEnergy, double temp)
{
	double val = std::exp(-deltaEnergy / temp);

	if (generateRand() < val)
		return true;
	else
		return false;
}

core::pose::PoseOP monteCarloOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs, double temp, int maxSteps,
		int reinitInterval)
{
	double currEn = getTotalEnergy(pose);
	double minEn = currEn;
	double tempEn;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP tempOptimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	if ((maxSteps <= 0) || (temp <= 0))
		return NULL;

	for (int i = 1; i <= maxSteps; i++)
	{
		(*mover)(working, optArgs);
		tempEn = getTotalEnergy(working);

		if (keepConformation(tempEn - currEn, temp))
		{
			tempOptimized->set_new_conformation(&(working->conformation()));
			currEn = tempEn;
		}
		else
		{
			working->set_new_conformation(&(tempOptimized->conformation()));
			tempEn = currEn;
		}

		if (tempEn < minEn)
		{
			optimized->set_new_conformation(&(working->conformation()));
			minEn = tempEn;
		}

		if ((reinitInterval >= 0) && (i % reinitInterval) == 0)
		{
			working->set_new_conformation(&(optimized->conformation()));
			tempOptimized->set_new_conformation(&(optimized->conformation()));
			tempEn = minEn;
			currEn = minEn;
		}
	}

	return optimized;
}

core::pose::PoseOP simulatedAnnealingOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs, double initTemp,
		double finalTemp, int numSteps, int maxSteps, int reinitInterval)
{
	double currEn = getTotalEnergy(pose);
	double minEn = currEn;
	double tempEn;
	if ((initTemp < finalTemp) || (numSteps > maxSteps) || (numSteps <= 0) || (maxSteps <= 0)
			|| (initTemp <= 0) || (finalTemp <= 0))
		return NULL;

	double currTemp = initTemp;
	int interSteps = maxSteps / numSteps;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP tempOptimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	for (int i = 1; i <= maxSteps; i++)
	{
		(*mover)(working, optArgs);
		tempEn = getTotalEnergy(working);

		if (keepConformation(tempEn - currEn, currTemp))
		{
			tempOptimized->set_new_conformation(&(working->conformation()));
			currEn = tempEn;
		}
		else
		{
			working->set_new_conformation(&(tempOptimized->conformation()));
			tempEn = currEn;
		}

		if (tempEn < minEn)
		{
			optimized->set_new_conformation(&(working->conformation()));
			minEn = tempEn;
		}

		if ((reinitInterval >= 0) && (i % reinitInterval) == 0)
		{
			working->set_new_conformation(&(optimized->conformation()));
			tempOptimized->set_new_conformation(&(optimized->conformation()));
			tempEn = minEn;
			currEn = minEn;
		}

		if ((i % interSteps) == 0)
		{
			currTemp = currTemp - ((initTemp - finalTemp) / (double) numSteps);
		}
	}

	return optimized;
}

core::pose::PoseOP greedyOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs, int maxSteps)
{
	double currEn = getTotalEnergy(pose);
	double tempEn;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	if (maxSteps <= 0)
		return NULL;

	for (int i = 1; i <= maxSteps; i++)
	{
		(*mover)(working, optArgs);
		tempEn = getTotalEnergy(working);

		if (tempEn < currEn)
		{
			optimized->set_new_conformation(&(working->conformation()));
			currEn = tempEn;
		}
		else
		{
			working->set_new_conformation(&(optimized->conformation()));
		}
	}

	return optimized;
}

core::pose::PoseOP sequentialOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs)
{
	core::pose::PoseOP acc = new core::pose::Pose();

	double initTemp = 4.0;
	double finalTemp = 0.01;
	int numSteps = 100;
	int maxSteps = 0;
	int factor = 5000;

	for (int i = 1; i <= pose->n_residue(); i++)
	{
		maxSteps = (int) std::max((int) ((factor * log(i)) + 1), numSteps);
		acc->append_residue_by_bond(pose->residue(i), true);
		core::pose::PoseOP tempPose = simulatedAnnealingOptimization(acc, mover, optArgs, initTemp,
				finalTemp, numSteps, maxSteps);
		//core::pose::PoseOP tempPose = monteCarloOptimization(acc, mover, optArgs, finalTemp,
		//		maxSteps);
		if (tempPose.get() == NULL)
		{
			return NULL;
		}
		acc->set_new_conformation(&(tempPose->conformation()));
	}

	return acc;
}

}
; /* namespace lbcpp */

