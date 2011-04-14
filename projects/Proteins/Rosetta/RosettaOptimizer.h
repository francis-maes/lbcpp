/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaOptimizer.h
 */

#ifndef LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_
#define LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_

#include "RosettaUtils.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#undef T
#include <core/conformation/Conformation.hh>
#define T JUCE_T

namespace lbcpp
{

/*
 * Performs greedy optimization on the pose object.
 * @param pose the initial conformation
 * @param mover pointer to the perturbation function used to modify the conformation
 * at each step
 * @param optArgs the arguments of the function, see the perturbation function
 * for more information. Default = NULL.
 * @param maxSteps number of steps to perform before stopping the optimization.
 * Default = 50000.
 * @return the new conformation
 */
core::pose::PoseOP greedyOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs = NULL, int maxSteps = 50000);

/*
 * Performs Monte Carlo optimization on the pose object.
 * @param pose the initial conformation
 * @param mover pointer to the perturbation function used to modify the conformation
 * at each step
 * @param optArgs the arguments of the function, see the perturbation function
 * for more information. Default = NULL.
 * @param temp the temperature used in Monte Carlo optimization. In fact, temp
 * represents the product k_b*T. Default = 1.0.
 * @param maxSteps number of steps to perform before stopping the optimization
 * Default = 50000.
 * @param reinitInterval number of steps to perform before resetting working conformation
 * to the lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
 * Default = 10000.
 * @return the new conformation
 */
core::pose::PoseOP monteCarloOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs = NULL, double temp = 1.0, int maxSteps =
		50000, int reinitInterval = 10000);

/*
 * Performs simulated annealing on the pose object.
 * @param pose the initial conformation
 * @param mover pointer to the perturbation function used to modify the conformation
 * at each step
 * @param optArgs the arguments of the function, see the perturbation function
 * for more information. Default = NULL.
 * @param initTemp the initial temperature used in simulated annealing. In fact, initTemp
 * represents the product k_b*T used in the first step. Default = 4.0.
 * @param finalTemp the initial temperature used in simulated annealing. In fact, initTemp
 * represents the product k_b*T used in the first step. Default = 0.01.
 * @param numSteps temperature decreases by step. numSteps represents the number of steps the
 * algorithm uses to decrease the temperature from initTemp to finalTemp.
 * @param maxSteps number of steps to perform before stopping the optimization
 * Default = 50000.
 * @param reinitInterval number of steps to perform before resetting working conformation
 * to the lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
 * Default = 10000.
 * @return the new conformation
 */
core::pose::PoseOP simulatedAnnealingOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs = NULL,
		double initTemp = 4.0, double finalTemp = 0.01, int numSteps = 100, int maxSteps = 50000,
		int reinitInterval = 10000);

core::pose::PoseOP sequentialOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs);

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_
