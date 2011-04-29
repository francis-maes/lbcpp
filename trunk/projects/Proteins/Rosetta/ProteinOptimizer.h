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
  /**
   * Initializes the ProteinOptimizer without any output.
   * @param seedForRandom seed used to initialize the random generator.
   */
  ProteinOptimizer(long long seedForRandom = 0)
  {
    context = NULL;
    name = (String)"Default";
    frequencyVerbosity = 0.1;
    verbosity = false;
    nameScopesSet = false;
    randomGenerator = new RandomGenerator(seedForRandom);
    saveToFile = false;
    numOutputFiles = 0;
  }

  /**
   * Initializes the ProteinOptimizer with callbacks.
   * @param c a pointer to the execution context used for the callbacks.
   * @param n the name of the protein optimized. (can be changed).
   * @param f the frequency you want the callbacks to appear in the
   * optimization. 0.1 gives 10 trace callbacks.
   * @param outputDirectory the directory where to put the saved files.
   * @param numOutputFiles the number of output files stored in outputDirectory.
   * @param seedForRandom seed used to initialize the random generator.
   */
  ProteinOptimizer(ExecutionContextPtr context, String name, double frequencyCallback,
      File outputDirectory, int numOutputFiles, long long seedForRandom = 0)
  {
    if (context.get() != NULL)
    {
      this->context = context;
      if (name.isEmpty())
        this->name = (String)"Default";
      else
        this->name = name;
      if ((frequencyCallback < 0) || (frequencyCallback > 1))
        frequencyVerbosity = 0.1;
      else
        frequencyVerbosity = frequencyCallback;
      verbosity = true;

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
    }
    else
    {
      this->context = NULL;
      this->name = (String)"Default";
      frequencyVerbosity = 0.1;
      this->numOutputFiles = 1;
      verbosity = false;
      saveToFile = false;
    }
    nameScopesSet = false;
    randomGenerator = new RandomGenerator(seedForRandom);
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

  virtual core::pose::PoseOP apply(core::pose::PoseOP& pose, ProteinMoverPtr& mover)=0;

protected:
  bool verbosity;
  double frequencyVerbosity;
  String name;
  ExecutionContextPtr context;
  bool nameScopesSet;
  std::vector<String> nameScopes;
  RandomGeneratorPtr randomGenerator;
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
  bool keepConformation(double deltaEnergy, double temperature)
  {
    double val = std::exp(-deltaEnergy / temperature);

    if (randomGenerator->sampleDouble() < val)
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
  void initializeCallbacks(const std::vector<String>& names, double energy)
  {
    if (verbosity)
    {
      nameScopesSet = true;
      nameScopes = names;
      String nameScope(nameScopes.at(0));
      nameScope += name;
      context->enterScope(nameScope);
      context->resultCallback(T("Protein name"), name);
      context->resultCallback(T("Initial Energy"), Variable(energy));
      context->enterScope(nameScopes.at(1));
    }
  }

  /**
   * Finalizes lbcpp callbacks.
   * @param en final energy of the conformation.
   */
  void finalizeCallbacks(double energy)
  {
    if (verbosity)
    {
      if (nameScopesSet)
      {
        context->leaveScope(); //leave Energies
        context->resultCallback(T("Final Energy"), Variable(energy));
        context->leaveScope(Variable(energy));
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
  void callback(const std::vector<Variable>& resultCallbackValues, Variable returnValue,
      int maxStepsNumber = 0)
  {
    if (verbosity)
    {
      if (nameScopesSet)
      {
        context->progressCallback(new ProgressionState(
            (double)resultCallbackValues.at(0).getInteger(), (double)maxStepsNumber, T("Iterations")));
        context->enterScope(nameScopes.at(2));
        for (int i = 0; i < resultCallbackValues.size(); i++)
          context->resultCallback(nameScopes.at(i + 3), Variable(resultCallbackValues.at(i)));
        context->leaveScope(returnValue);
      }
      else
        context->errorCallback(T("nameScopes undefined."));
    }
  }

};

extern ProteinOptimizerPtr proteinGreedyOptimizer();
extern ProteinOptimizerPtr proteinMonteCarloOptimizer();
extern ProteinOptimizerPtr proteinSimulatedAnnealingOptimizer();
extern ProteinOptimizerPtr proteinSequentialOptimizer();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_OPTIMIZER_H_
