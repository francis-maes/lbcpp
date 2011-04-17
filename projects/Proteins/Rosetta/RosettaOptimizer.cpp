/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaOptimizer.cpp
 */

#include "RosettaOptimizer.h"

namespace lbcpp
{

void RosettaOptimizer::initCallbacks(std::vector<juce::String> n, double en)
{
	if (verbosity)
	{
		nameScopesSet = true;
		nameScopes = n;
		juce::String nameScope(nameScopes.at(0));
		nameScope += name;
		context->enterScope(nameScope);
		context->enterScope(T("Initial energy"));
		context->resultCallback(T("Initial Energy"), Variable(en));
		context->leaveScope(Variable(en));
		context->enterScope(nameScopes.at(1));
	}
}

void RosettaOptimizer::finalizeCallbacks(double en)
{
	if (verbosity)
	{
		context->leaveScope(); //leave Energies
		context->enterScope(T("Final energy"));
		context->resultCallback(T("Final Energy"), Variable(en));
		context->leaveScope(Variable(en));
		context->leaveScope(Variable(en));
		nameScopesSet = false;
	}
}

void RosettaOptimizer::callback(std::vector<Variable> vals)
{
	if (verbosity)
	{
		if (nameScopesSet)
		{
			context->enterScope(nameScopes.at(2));
			for (int i = 0; i < vals.size(); i++)
				context->resultCallback(nameScopes.at(i + 3), Variable(vals.at(i)));
			context->leaveScope();
		}
		else
			context->errorCallback(T("nameScopes undefined."));
	}
}

RosettaOptimizer::RosettaOptimizer()
{
	context = NULL;
	name = (juce::String) "Default";
	freqVerb = 0.1;
	verbosity = false;
	nameScopesSet = false;
}

RosettaOptimizer::RosettaOptimizer(ExecutionContextPtr c, juce::String n, double f)
{
	if (c.get() != NULL)
	{
		context = c;
		if (n.isEmpty())
			name = (juce::String) "Default";
		else
			name = n;
		if ((f < 0) || (f > 1))
			freqVerb = 0.1;
		else
			freqVerb = f;
		verbosity = true;
	}
	else
	{
		context = NULL;
		name = (juce::String) "Default";
		freqVerb = 0.1;
		verbosity = false;
	}
	nameScopesSet = false;
}

RosettaOptimizer::~RosettaOptimizer()
{
}

juce::String RosettaOptimizer::getProteinName()
{
	juce::String tmp(name);
	return tmp;
}

void RosettaOptimizer::setProteinName(juce::String n)
{
	name = (juce::String) n;
}

bool RosettaOptimizer::keepConformation(double deltaEnergy, double temp)
{
	double val = std::exp(-deltaEnergy / temp);

	if (generateRand() < val)
		return true;
	else
		return false;
}

core::pose::PoseOP RosettaOptimizer::monteCarloOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs, double temp,
		int maxSteps, int timesReinit)
{
	double currEn = getTotalEnergy(pose);
	double minEn = currEn;
	double tempEn = currEn;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP tempOptimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	int reinitInterval = -1;
	if (timesReinit > 0)
		reinitInterval = maxSteps / timesReinit;

	if ((maxSteps <= 0) || (temp <= 0))
		return NULL;

	// Init verbosity
	juce::String ns("Monte carlo optim : ");
	int intervVerb = (int) std::ceil(maxSteps * freqVerb);
	std::vector<Variable> vals;
	if (verbosity)
	{
		std::vector<juce::String> v;
		v.push_back(ns);
		v.push_back(T("Energies"));
		v.push_back(T("Energy"));
		v.push_back(T("Step"));
		v.push_back(T("Minimal energy"));
		v.push_back(T("Temporary energy"));
		v.push_back(T("Temperature"));
		initCallbacks(v, minEn);
		vals.push_back(Variable((int) 0));
		vals.push_back(Variable(minEn));
		vals.push_back(Variable(tempEn));
		vals.push_back(Variable(temp));
		callback(vals);
	}

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

		if ((reinitInterval > 0) && (i % reinitInterval) == 0)
		{
			working->set_new_conformation(&(optimized->conformation()));
			tempOptimized->set_new_conformation(&(optimized->conformation()));
			tempEn = minEn;
			currEn = minEn;
		}

		// Verbosity
		if (verbosity && (((i % intervVerb) == 0) || (i == maxSteps)))
		{
			vals.at(0) = Variable((int) i);
			vals.at(1) = Variable(minEn);
			vals.at(2) = Variable(currEn);
			vals.at(3) = Variable(temp);
			callback(vals);
		}
	}
	// Verbosity
	if (verbosity)
		finalizeCallbacks(minEn);

	return optimized;
}

core::pose::PoseOP RosettaOptimizer::simulatedAnnealingOptimization(core::pose::PoseOP pose,
		void(*mover)(core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs,
		double initTemp, double finalTemp, int numSteps, int maxSteps, int timesReinit)
{
	double currEn = getTotalEnergy(pose);
	double minEn = currEn;
	double tempEn = currEn;
	if ((initTemp < finalTemp) || (numSteps > maxSteps) || (numSteps <= 0) || (maxSteps <= 0)
			|| (initTemp <= 0) || (finalTemp <= 0))
		return NULL;

	int reinitInterval = -1;
	if (timesReinit > 0)
		reinitInterval = maxSteps / timesReinit;

	double currTemp = initTemp;
	int interSteps = maxSteps / numSteps;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP tempOptimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	// Init verbosity
	juce::String ns("Simulated annealing optim : ");
	int intervVerb = (int) std::ceil(maxSteps * freqVerb);
	std::vector<Variable> vals;
	if (verbosity)
	{
		std::vector<juce::String> v;
		v.push_back(ns);
		v.push_back(T("Energies"));
		v.push_back(T("Energy"));
		v.push_back(T("Step"));
		v.push_back(T("Minimal energy"));
		v.push_back(T("Temporary energy"));
		v.push_back(T("Temperature"));
		initCallbacks(v, minEn);
		vals.push_back(Variable((int) 0));
		vals.push_back(Variable(minEn));
		vals.push_back(Variable(tempEn));
		vals.push_back(Variable(currTemp));
		callback(vals);
	}

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

		if ((reinitInterval > 0) && (i % reinitInterval) == 0)
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

		// Verbosity
		if (verbosity && (((i % intervVerb) == 0) || (i == maxSteps)))
		{
			vals.at(0) = Variable((int) i);
			vals.at(1) = Variable(minEn);
			vals.at(2) = Variable(currEn);
			vals.at(3) = Variable(currTemp);
			callback(vals);
		}
	}
	// Verbosity
	if (verbosity)
		finalizeCallbacks(minEn);

	return optimized;
}

core::pose::PoseOP RosettaOptimizer::greedyOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs, int maxSteps)
{
	// Initialization
	double minEn = getTotalEnergy(pose);
	double tempEn = minEn;
	core::pose::PoseOP optimized = new core::pose::Pose((*pose));
	core::pose::PoseOP working = new core::pose::Pose((*pose));

	if (maxSteps <= 0)
		return NULL;

	// Init verbosity
	juce::String ns("Greedy optim : ");
	int intervVerb = (int) std::ceil(maxSteps * freqVerb);
	std::vector<Variable> vals;
	if (verbosity)
	{
		std::vector<juce::String> v;
		v.push_back(ns);
		v.push_back(T("Energies"));
		v.push_back(T("Energy"));
		v.push_back(T("Step"));
		v.push_back(T("Minimal energy"));
		v.push_back(T("Temporary energy"));
		initCallbacks(v, minEn);
		vals.push_back(Variable((int) 0));
		vals.push_back(Variable(minEn));
		vals.push_back(Variable(tempEn));
		callback(vals);
	}

	for (int i = 1; i <= maxSteps; i++)
	{
		(*mover)(working, optArgs);
		tempEn = getTotalEnergy(working);

		if (tempEn < minEn)
		{
			optimized->set_new_conformation(&(working->conformation()));
			minEn = tempEn;
		}
		else
		{
			working->set_new_conformation(&(optimized->conformation()));
		}

		// Verbosity
		if (verbosity && (((i % intervVerb) == 0) || (i == maxSteps)))
		{
			vals.at(0) = Variable((int) i);
			vals.at(1) = Variable(minEn);
			vals.at(2) = Variable(tempEn);
			callback(vals);
		}
	}

	// Verbosity
	if (verbosity)
		finalizeCallbacks(minEn);

	// Return
	return optimized;
}

core::pose::PoseOP RosettaOptimizer::sequentialOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs)
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

