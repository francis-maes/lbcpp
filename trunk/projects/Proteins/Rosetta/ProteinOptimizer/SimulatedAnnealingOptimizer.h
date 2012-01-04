/*-----------------------------------------.---------------------------------.
| Filename: SimulatedAnnealingOptimizer.h  | SimulatedAnnealingOptimizer     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  22:44:14         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_SIMULATED_ANNEALING_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_SIMULATED_ANNEALING_OPTIMIZER_H_

# include "precompiled.h"
# include "ProteinOptimizer.h"
# include "../Data/PoseMover.h"

namespace lbcpp
{
class ProteinSimulatedAnnealingOptimizer;
typedef ReferenceCountedObjectPtr<ProteinSimulatedAnnealingOptimizer>
    ProteinSimulatedAnnealingOptimizerPtr;

class ProteinSimulatedAnnealingOptimizer : public ProteinOptimizer
{
public:
  ProteinSimulatedAnnealingOptimizer(double initialTemperature, double finalTemperature,
      int numberDecreasingSteps, int maxSteps, int timesReinitialization, String name =
          juce::String("Default"), double frequencyCallback = 0.01, int numOutputFiles = -1,
      File outputDirectory = juce::File()) :
    ProteinOptimizer(name, frequencyCallback, numOutputFiles, outputDirectory), initialTemperature(
        initialTemperature), finalTemperature(finalTemperature), numberDecreasingSteps(
        numberDecreasingSteps), maxSteps(maxSteps), timesReinitialization(timesReinitialization)
  {
  }

  ProteinSimulatedAnnealingOptimizer() {}

  virtual void apply(ExecutionContext& context, RosettaWorkerPtr& worker,
                    const RandomGeneratorPtr& random, DenseDoubleVectorPtr& energiesAtIteration)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    simulatedAnnealingOptimization(context, worker, random, energiesAtIteration, initialTemperature,
        finalTemperature, numberDecreasingSteps, maxSteps, timesReinitialization);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

#ifdef LBCPP_PROTEIN_ROSETTA
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
  void simulatedAnnealingOptimization(ExecutionContext& context, RosettaWorkerPtr& worker,
      const RandomGeneratorPtr& random, DenseDoubleVectorPtr& energiesAtIteration,
      double initialTemperature = 4.0, double finalTemperature = 0.01, int numberDecreasingSteps =
          50, int maxSteps = 50000, int timesReinitialization = 5)
  {
    core::pose::PoseOP pose;
    worker->getPose(pose);
    energiesAtIteration = new DenseDoubleVector(0, 0.0);

    double currentEnergy = getConformationScore(pose, fullAtomEnergy);
    double minimumEnergy = currentEnergy;
    double temporaryEnergy = currentEnergy;

    if ((initialTemperature < finalTemperature) || (numberDecreasingSteps > maxSteps)
        || (numberDecreasingSteps <= 0) || (maxSteps <= 0) || (initialTemperature <= 0)
        || (finalTemperature <= 0))
      {jassert(false);}

    int reinitializationInterval = -1;
    if (timesReinitialization > 0)
      reinitializationInterval = juce::jlimit(1, maxSteps, maxSteps / timesReinitialization);

    double currentTemperature = initialTemperature;
    int intervalDecreasingTemperature = juce::jmax(1, maxSteps / numberDecreasingSteps);
    double bCoeff = initialTemperature;
    double aCoeff = (finalTemperature - initialTemperature) / maxSteps;
    core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
    core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
    core::pose::PoseOP workingPose = new core::pose::Pose((*pose));
    worker->setPose(workingPose);

    // ratio decreasing and accepted movers
    int numMoversSinceLastCheck = 0;
    int numAcceptedMovers = 0;
    int numDecreasingMovers = 0;

    // Init verbosity
    String nameEnglobingScope("Simulated annealing optimization : ");
    int intervalVerbosity =
        juce::jlimit(1, maxSteps, (int)std::ceil(maxSteps * frequencyVerbosity));
    std::vector<Variable> resultCallbackValues;
    if (verbosity)
    {
      std::vector<String> englobingScopesNames;
      englobingScopesNames.push_back(nameEnglobingScope);
      englobingScopesNames.push_back(T("Energies"));
      englobingScopesNames.push_back(T("Energy"));
      englobingScopesNames.push_back(T("Step"));
      englobingScopesNames.push_back(T("Minimal energy"));
      englobingScopesNames.push_back(T("Temporary energy"));
      englobingScopesNames.push_back(T("Temperature"));
      englobingScopesNames.push_back(T("Ratio accepted movers"));
      englobingScopesNames.push_back(T("Ratio energy decreasing movers"));
      initializeCallbacks(context, englobingScopesNames, minimumEnergy);
      resultCallbackValues.push_back(Variable((int)0));
      resultCallbackValues.push_back(Variable(minimumEnergy));
      resultCallbackValues.push_back(Variable(temporaryEnergy));
      resultCallbackValues.push_back(Variable(currentTemperature));
      resultCallbackValues.push_back(Variable((double)1.0));
      resultCallbackValues.push_back(Variable((double)1.0));
      callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
    }
    int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
    String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
    int indexOutputFile = 0;

    if (saveToFile)
    {
      ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".pdb");
      File temporaryFile(temporaryOutputFileName);
      protein->saveToPDBFile(context, temporaryFile);
      indexOutputFile++;
    }

    for (int i = 1; i <= maxSteps; i++)
    {
      worker->update();
      PoseMoverPtr mover = worker->sample(context, random).getObjectAndCast<PoseMover> ();

      mover->move(workingPose);
      worker->energies(NULL, &temporaryEnergy, NULL);

      numMoversSinceLastCheck++;

      if (keepConformation(random, temporaryEnergy - currentEnergy, currentTemperature))
      {
        (*temporaryOptimizedPose) = (*workingPose);
        if (temporaryEnergy < currentEnergy)
          numDecreasingMovers++;
        currentEnergy = temporaryEnergy;
        numAcceptedMovers++;
      }
      else
      {
        (*workingPose) = (*temporaryOptimizedPose);
        temporaryEnergy = currentEnergy;
      }

      if (temporaryEnergy < minimumEnergy)
      {
        (*optimizedPose) = (*workingPose);
        minimumEnergy = temporaryEnergy;
      }

      if (((reinitializationInterval > 0) && (i % reinitializationInterval) == 0) || (i == maxSteps))
      {
        (*workingPose) = (*optimizedPose);
        (*temporaryOptimizedPose) = (*optimizedPose);
        temporaryEnergy = minimumEnergy;
        currentEnergy = minimumEnergy;
      }

      if ((i % intervalDecreasingTemperature) == 0)
        currentTemperature = aCoeff * i + bCoeff;

      // Verbosity
      if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
      {
        energiesAtIteration->appendValue(minimumEnergy);
        resultCallbackValues.at(0) = Variable((int)i);
        resultCallbackValues.at(1) = Variable(minimumEnergy);
        resultCallbackValues.at(2) = Variable(currentEnergy);
        resultCallbackValues.at(3) = Variable(currentTemperature);
        if (numMoversSinceLastCheck == 0)
          resultCallbackValues.at(4) = Variable((double)0.0);
        else
          resultCallbackValues.at(4) = Variable((double)numAcceptedMovers
              / (double)numMoversSinceLastCheck);
        if (numAcceptedMovers == 0)
          resultCallbackValues.at(5) = Variable((double)0.0);
        else
          resultCallbackValues.at(5) = Variable((double)numDecreasingMovers
              / (double)numAcceptedMovers);
        callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
        numMoversSinceLastCheck = 0;
        numDecreasingMovers = 0;
        numAcceptedMovers = 0;
      }

      if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
      {
        ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
        String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".pdb");
        File temporaryFile(temporaryOutputFileName);
        protein->saveToPDBFile(context, temporaryFile);
        indexOutputFile++;
      }
    }
    // Verbosity
    if (verbosity)
      finalizeCallbacks(context, minimumEnergy);
  }
#endif // LBCPP_PROTEIN_ROSETTA

private:
  friend class ProteinSimulatedAnnealingOptimizerClass;

  double initialTemperature;
  double finalTemperature;
  int numberDecreasingSteps;
  int maxSteps;
  int timesReinitialization;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_SIMULATED_ANNEALING_OPTIMIZER_H_
