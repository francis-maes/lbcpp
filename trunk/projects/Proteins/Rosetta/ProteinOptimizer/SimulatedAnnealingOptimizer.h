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
# include "../ProteinOptimizer.h"
# include "../Sampler.h"
# include "../ProteinMover.h"

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
      englobingScopesNames.push_back(T("Minimal energy (log10)"));
      englobingScopesNames.push_back(T("Temporary energy (log10)"));
      initializeCallbacks(context, englobingScopesNames, minimumEnergy);
      resultCallbackValues.push_back(Variable((int)0));
      resultCallbackValues.push_back(Variable(minimumEnergy));
      resultCallbackValues.push_back(Variable(temporaryEnergy));
      resultCallbackValues.push_back(Variable(currentTemperature));
      double logMinimumEnergy = minimumEnergy >= 1 ? log10(minimumEnergy) : -log10(std::abs(minimumEnergy - 2));
      double logTemporaryEnergy = temporaryEnergy >= 1 ? log10(temporaryEnergy) : -log10(std::abs(temporaryEnergy - 2));
      resultCallbackValues.push_back(Variable(logMinimumEnergy));
      resultCallbackValues.push_back(Variable(logTemporaryEnergy));
      callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
    }
    int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
    String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
    int indexOutputFile = 0;

    if (saveToFile)
    {
      ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
      File temporaryFile(temporaryOutputFileName);
      protein->saveToXmlFile(context, temporaryFile);
      indexOutputFile++;
    }

    for (int i = 1; i <= maxSteps; i++)
    {
      worker->update();
      ProteinMoverPtr mover = worker->sample(context, random).getObjectAndCast<ProteinMover> ();

      mover->move(workingPose);
      worker->energies(NULL, &temporaryEnergy, NULL);

      if (keepConformation(random, temporaryEnergy - currentEnergy, currentTemperature))
      {
        (*temporaryOptimizedPose) = (*workingPose);
        currentEnergy = temporaryEnergy;
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
        resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
        resultCallbackValues.at(5) = Variable(log10(currentEnergy));
        callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
      }

      if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
      {
        ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
        String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
        File temporaryFile(temporaryOutputFileName);
        protein->saveToXmlFile(context, temporaryFile);
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
