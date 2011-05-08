/*-----------------------------------------.---------------------------------.
| Filename:  ProteinOptimizer.h            | ProteinOptimizer                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_OPTIMIZER_H_

# include "precompiled.h"

# include "RosettaUtils.h"
# include "ProteinMover.h"
# include <iostream>
# include <cmath>
# include <algorithm>
# include <vector>
# include "Sampler/ProteinMoverSampler.h"

# undef T
#  include <core/conformation/Conformation.hh>
# define T JUCE_T

namespace lbcpp
{

class ProteinOptimizer;
typedef ReferenceCountedObjectPtr<ProteinOptimizer> ProteinOptimizerPtr;

class ProteinOptimizer: public Object
{
public:
  ProteinOptimizer()
  {
    name = String("Default");
    frequencyVerbosity = 0.01;
    verbosity = true;
    saveToFile = false;
    this->numOutputFiles = 1;
    nameScopesSet = false;
  }

  /**
   * Initializes the ProteinOptimizer with callbacks.
   * @param name the name of the protein optimized. (can be changed).
   * @param frequencyCallbacks the frequency you want the callbacks to appear in the
   * optimization. 0.01 gives 100 trace callbacks. If <= 0, no verbosity.
   * @param numOutputFiles the number of output files stored in outputDirectory.
   * @param outputDirectory the directory where to put the saved files.
   */
  ProteinOptimizer(String name, double frequencyCallback = 0.01, int numOutputFiles = -1,
      File outputDirectory = juce::File()) :
    name(name)
  {
    if (frequencyCallback > 0)
    {
      frequencyVerbosity = frequencyCallback > 1 ? 0.01 : frequencyCallback;
      verbosity = true;
    }
    else
    {
      frequencyVerbosity = 0.01;
      verbosity = false;
    }

    if (numOutputFiles <= 0)
    {
      saveToFile = false;
      this->numOutputFiles = 1;
    }
    else
    {
      this->numOutputFiles = numOutputFiles;
      saveToFile = true;
      this->outputDirectory = outputDirectory;
      if (!this->outputDirectory.exists())
        this->outputDirectory.createDirectory();
    }
    nameScopesSet = false;
  }

  /**
   * Returns the protein name.
   * @return protein name.
   */
  String getProteinName()
  {
    return name;
  }

  /**
   * Sets the protein name (used in the callbacks).
   * @param n the new protein name.
   */
  void setProteinName(String name)
  {
    this->name = name;
  }

  /**
   * Sets the verbosity level to true or false.
   * @param v new verbosity level.
   */
  void setVerbosity(bool verbosity)
  {
    this->verbosity = verbosity;
  }

  /**
   * Gets the actual verbosity level.
   * @return the verbosity level.
   */
  bool getVerbosity()
  {
    return verbosity;
  }

  /**
   * Sets the new frequency of trace callbacks. It depends on the number
   * of optimization steps performed by the optmizer. For example, if you
   * give 0.1 for this frequency, there will be 10 trace callbacks.
   */
  void setFrequency(double frequencyCallback)
  {
    frequencyVerbosity = frequencyCallback;
  }

  /**
   * Returns the actual trace callbacks frequency.
   * @return the actual trace callbacks frequency.
   */
  double getFrequency()
  {
    return frequencyVerbosity;
  }

  virtual core::pose::PoseOP apply(core::pose::PoseOP& pose, ProteinMoverSamplerPtr& sampler,
      ExecutionContext& context, RandomGeneratorPtr& random)=0;

protected:
  friend class ProteinOptimizerClass;
  bool verbosity;
  double frequencyVerbosity;
  String name;
  bool nameScopesSet;
  std::vector<String> nameScopes;
  bool saveToFile;
  juce::File outputDirectory;
  int numOutputFiles;

  /**
   * Function that performs the monte carlo selection based on the
   * difference in the energies.
   * @param deltaEnergy the difference in the energies.
   * @param temperature the normalized temperature, i.e. kT.
   * @return a boolean, if true, keep the conformation.
   */
  bool keepConformation(RandomGeneratorPtr& random, double deltaEnergy, double temperature)
  {
    double val = std::exp(-deltaEnergy / temperature);

    if (random->sampleDouble() < val)
      return true;
    else
      return false;
  }

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
  void initializeCallbacks(ExecutionContext& context, const std::vector<String>& names,
      double energy)
  {
    if (verbosity)
    {
      nameScopesSet = true;
      nameScopes = names;
      String nameScope(nameScopes.at(0));
      nameScope += name;
      context.enterScope(nameScope);
      context.resultCallback(T("Protein name"), name);
      context.resultCallback(T("Initial Energy"), Variable(energy));
      context.enterScope(nameScopes.at(1));
    }
  }

  /**
   * Finalizes lbcpp callbacks.
   * @param en final energy of the conformation.
   */
  void finalizeCallbacks(ExecutionContext& context, double energy)
  {
    if (verbosity)
    {
      if (nameScopesSet)
      {
        context.leaveScope(); //leave Energies
        context.resultCallback(T("Final Energy"), Variable(energy));
        context.leaveScope(Variable(energy));
        nameScopesSet = false;
      }
    }
  }

  /**
   * Callbacks. You only need to provide a vector of Variable that contain the things
   * you want to appear in the trace. The size of resultCallbackValues must the same as (the size of n)-3
   * (see initializeCallbacks).
   * @param resultCallbackValues the values to add in the trace (beware of the number of variables). First
   * variable should the iteration number.
   */
  void callback(ExecutionContext& context, const std::vector<Variable>& resultCallbackValues,
      Variable returnValue, int maxStepsNumber = 0)
  {
    if (verbosity)
    {
      if (nameScopesSet)
      {
        context.progressCallback(new ProgressionState(
            (double)resultCallbackValues.at(0).getInteger(), (double)maxStepsNumber,
            T("Iterations")));
        context.enterScope(nameScopes.at(2));
        for (int i = 0; i < resultCallbackValues.size(); i++)
          context.resultCallback(nameScopes.at(i + 3), Variable(resultCallbackValues.at(i)));
        context.leaveScope(returnValue);
      }
      else
        context.errorCallback(T("nameScopes undefined."));
    }
  }

};

extern ProteinOptimizerPtr proteinGreedyOptimizer();
extern ProteinOptimizerPtr proteinMonteCarloOptimizer();
extern ProteinOptimizerPtr proteinSimulatedAnnealingOptimizer();
extern ProteinOptimizerPtr proteinSequentialOptimizer();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_OPTIMIZER_H_
