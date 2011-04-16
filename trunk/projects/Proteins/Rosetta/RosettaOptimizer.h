/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaOptimizer.h
 */

#ifndef LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_
#define LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_

//#include <lbcpp/Core/Variable.h>
//#include <lbcpp/Execution/ExecutionContext.h>

#include "RosettaUtils.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

#undef T
#include <core/conformation/Conformation.hh>
#define T JUCE_T

namespace lbcpp
{
/*namespace options
 {
 class callbackOptions;
 typedef ReferenceCountedObjectPtr<callbackOptions> callbackOptionsPtr;

 class callbackOptions
 {
 private:
 //ExecutionContextPtr context;
 std::vector<Variable>* vars;

 public:
 callbackOptions()
 {
 vars = new std::vector<Variable>;
 }
 ~callbackOptions()
 {
 delete (vars);
 }

 void add(const Variable& i)
 {
 Variable tmp(i);
 vars->push_back(tmp);
 }
 Variable get(int index)
 {
 Variable tmp(vars->at(index));
 return tmp;
 }
 };

 }
 ; // namespace options
 */

/*
 * Performs greedy optimization on the pose object.
 * @param pose the initial conformation
 * @param mover pointer to the perturbation function used to modify the conformation
 * at each step
 * @param optArgs the arguments of the function, see the perturbation function
 * for more information. Default = NULL.
 * @param maxSteps number of steps to perform before stopping the optimization.
 * Default = 50000.
 * @param str options used to create the trace of the optimization. First argument =
 * name of the protein to optimize. Second = a double between 0 and 1, that specifies
 * the frequency of the trace callbacks. It is given in the form of a juce::String also.
 * juce::String((double)0.2) for example. NULL if no trace desired, default.
 * @param context, the context used to create the trace. NULL if no trace desired, default.
 * @return the new conformation
 */
core::pose::PoseOP greedyOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs = NULL, int maxSteps = 50000,
		juce::String* opt = NULL, ExecutionContextPtr context = NULL);

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
 * @param timesReinit number of times the working conformation is resetted to the
 * lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
 * Default = 5.
 * @return the new conformation
 */
core::pose::PoseOP monteCarloOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs = NULL, double temp = 1.0, int maxSteps =
		50000, int timesReinit = 5);

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
 * @param timesReinit number of times the working conformation is resetted to the
 * lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
 * Default = 5.
 * @return the new conformation
 */
core::pose::PoseOP simulatedAnnealingOptimization(core::pose::PoseOP pose, void(*mover)(
		core::pose::PoseOP, std::vector<void*>*), std::vector<void*>* optArgs = NULL,
		double initTemp = 4.0, double finalTemp = 0.01, int numSteps = 100, int maxSteps = 50000,
		int timesReinit = 5);

core::pose::PoseOP sequentialOptimization(core::pose::PoseOP pose, void(*mover)(core::pose::PoseOP,
		std::vector<void*>*), std::vector<void*>* optArgs);

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTAOPTIMIZER_H_
