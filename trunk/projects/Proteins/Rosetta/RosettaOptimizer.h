/*-----------------------------------------.---------------------------------.
| Filename:  RosettaOptimizer.h            | RosettaOptimizer.h              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_OPTIMIZER_H_

# include "precompiled.h"

# include "RosettaUtils.h"
# include "RosettaMover.h"
# include <iostream>
# include <cmath>
# include <algorithm>
# include <vector>

# undef T
#  include <core/conformation/Conformation.hh>
# define T JUCE_T

namespace lbcpp
{

class RosettaOptimizer;
typedef ReferenceCountedObjectPtr<RosettaOptimizer> RosettaOptimizerPtr;

class RosettaOptimizer: public Object
{
public:
  /**
   * Initializes the RosettaOptimizer without any output.
   */
  RosettaOptimizer();

  /**
   * Initializes the RosettaOptimizer with callbacks.
   * @param c a pointer to the execution context used for the callbacks.
   * @param n the name of the protein optimized. (can be changed).
   * @param f the frequency you want the callbacks to appear in the
   * optimization. 0.1 gives 10 trace callbacks.
   */
  RosettaOptimizer(ExecutionContextPtr context, String name, double frequency);

  /**
   * Destructor.
   */
  ~RosettaOptimizer();

  /**
   * Returns the protein name.
   * @return protein name.
   */
  String getProteinName();

  /**
   * Sets the protein name (used in the callbacks).
   * @param n the new protein name.
   */
  void setProteinName(String name);

  /**
   * Sets the verbosity level to true or false.
   * @param v new verbosity level.
   */
  void setVerbosity(bool verbosity);

  /**
   * Gets the actual verbosity level.
   * @return the verbosity level.
   */
  bool getVerbosity();

  /**
   * Sets the new frequency of trace callbacks. It depends on the number
   * of optimization steps performed by the optmizer. For example, if you
   * give 0.1 for this frequency, there will be 10 trace callbacks.
   */
  void setFrequency(double frequency);

  /**
   * Returns the actual trace callbacks frequency.
   * @return the actual trace callbacks frequency.
   */
  double getFrequency();

  /*
   * Performs greedy optimization on the pose object.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @param maxSteps number of steps to perform before stopping the optimization.
   * Default = 50000.
   * @param str options used to create the trace of the optimization. First argument =
   * name of the protein to optimize. Second = a double between 0 and 1, that specifies
   * the frequency of the trace callbacks. It is given in the form of a String also.
   * String((double)0.2) for example. NULL if no trace desired, default.
   * @param context, the context used to create the trace. NULL if no trace desired, default.
   * @return the new conformation
   */
  core::pose::PoseOP greedyOptimization(core::pose::PoseOP pose, RosettaMoverPtr mover,
      int maxSteps = 50000);

  /*
   * Performs Monte Carlo optimization on the pose object.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @param temperature the temperature used in Monte Carlo optimization. In fact, temperature
   * represents the product k_b*T. Default = 1.0.
   * @param maxSteps number of steps to perform before stopping the optimization
   * Default = 50000.
   * @param timesReinitialization number of times the working conformation is resetted to the
   * lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
   * Default = 5.
   * @return the new conformation
   */
  core::pose::PoseOP monteCarloOptimization(core::pose::PoseOP pose, RosettaMoverPtr mover,
      double temperature = 1.0, int maxSteps = 50000, int timesReinitialization = 5);

  /*
   * Performs simulated annealing on the pose object.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @param initialTemperature the initial temperature used in simulated annealing. In fact, initialTemperature
   * represents the product k_b*T used in the first step. Default = 4.0.
   * @param finalTemperature the initial temperature used in simulated annealing. In fact, initialTemperature
   * represents the product k_b*T used in the first step. Default = 0.01.
   * @param numberDecreasingSteps temperature decreases by step. numberDecreasingSteps represents the number of steps the
   * algorithm uses to decrease the temperature from initialTemperature to finalTemperature.
   * @param maxSteps number of steps to perform before stopping the optimization
   * Default = 50000.
   * @param timesReinitialization number of times the working conformation is resetted to the
   * lowest-energy conformation found so far. If set to -1, no reinitialization is performed.
   * Default = 5.
   * @return the new conformation
   */
  core::pose::PoseOP simulatedAnnealingOptimization(core::pose::PoseOP pose, RosettaMoverPtr mover,
      double initialTemperature = 4.0, double finalTemperature = 0.01, int numberDecreasingSteps =
          100, int maxSteps = 50000, int timesReinitialization = 5);

  /*
   * Performs sequential simulation on the pose object. This function adds a residue
   * at each iteration and then performs optimization on the resulting protein object.
   * The purpose is to fold the protein as it was being cronstructed.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @return the new conformation
   */
  core::pose::PoseOP sequentialOptimization(core::pose::PoseOP pose, RosettaMoverPtr mover);

private:
  bool verbosity;
  double frequencyVerbosity;
  String name;
  ExecutionContextPtr context;
  bool nameScopesSet;
  std::vector<String> nameScopes;

  /**
   * Function that performs the monte carlo selection based on the
   * difference in the energies.
   * @param deltaEnergy the difference in the energies.
   * @param temperature the normalized temperature, i.e. kT.
   * @return a boolean, if true, keep the conformation.
   */
  bool keepConformation(double deltaEnergy, double temperature);

  /**
   * Initializes lbcpp callbacks.
   * @param names a vector containing all the names of the scopes used to store
   * the results and so on. First must be the name of the englobing scope. Second
   * the name of the scope englobing the results. Third the name of each scope
   * containing one result. The others depend on what you want to output. Thus, there
   * must be 3 + number of outputs for each conformation names in this vector. The fourth
   * variable should be the iteration number.
   * @param energy initial energy.
   * @param mover a pointer to the mover used to modify the protein object. This
   * will create a scope with tha name of the mover and with its parameters.
   */
  void initializeCallbacks(const std::vector<String>& names, double energy, int maxStepsNumber,
      RosettaMoverPtr mover = NULL);

  /**
   * Finalizes lbcpp callbacks.
   * @param en final energy of the conformation.
   */
  void finalizeCallbacks(double energy);

  /**
   * Callbacks. You only need to provide a vector of Variable that contain the things
   * you want to appear in the trace. The size of resultCallbackValues must the same as (the size of n)-3
   * (see initializeCallbacks).
   * @param resultCallbackValues the values to add in the trace (beware of the number of variables). First
   * variable should the iteration number.
   */
  void callback(const std::vector<Variable>& resultCallbackValues, Variable returnValue,
      int maxStepsNumber);
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_OPTIMIZER_H_
