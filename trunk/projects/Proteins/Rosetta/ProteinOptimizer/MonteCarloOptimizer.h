/*-----------------------------------------.---------------------------------.
| Filename: MonteCarloOptimizer.h          | MonteCarloOptimizer             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  22:44:37         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_MONTE_CARLO_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_MONTE_CARLO_OPTIMIZER_H_

# include "precompiled.h"
# include "../ProteinOptimizer.h"
# include "../Sampler.h"
# include "../ProteinMover.h"

namespace lbcpp
{

class ProteinMonteCarloOptimizer;
typedef ReferenceCountedObjectPtr<ProteinMonteCarloOptimizer> ProteinMonteCarloOptimizerPtr;

class ProteinMonteCarloOptimizer : public ProteinOptimizer
{
public:
  ProteinMonteCarloOptimizer(double temperature, int maxSteps, int timesReinitialization,
      String name = juce::String("Default"), double frequencyCallback = 0.01, int numOutputFiles =
          -1, File outputDirectory = juce::File()) :
    ProteinOptimizer(name, frequencyCallback, numOutputFiles, outputDirectory), temperature(
        temperature), maxSteps(maxSteps), timesReinitialization(timesReinitialization)
  {
  }

  ProteinMonteCarloOptimizer() {}

  virtual void apply(ExecutionContext& context, RosettaWorkerPtr& worker,
      const RandomGeneratorPtr& random, DenseDoubleVectorPtr& energiesAtIteration)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    monteCarloOptimization(context, worker, random, energiesAtIteration, temperature, maxSteps,
        timesReinitialization);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

#ifdef LBCPP_PROTEIN_ROSETTA
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
  core::pose::PoseOP monteCarloOptimization(ExecutionContext& context, RosettaWorkerPtr& worker,
      const RandomGeneratorPtr& random, DenseDoubleVectorPtr& energiesAtIteration,
      double temperature = 1.0, int maxSteps = 50000, int timesReinitialization = 5)
  {
//    core::pose::PoseOP pose;
//    worker->getPose(pose);
//
//    double currentEnergy = getConformationScore(pose, fullAtomEnergy);
//    double minimumEnergy = currentEnergy;
//    double temporaryEnergy = currentEnergy;
//    core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
//    core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
//    core::pose::PoseOP workingPose = new core::pose::Pose((*pose));
//
//    if ((maxSteps <= 0) || (temperature <= 0))
//    {
//      std::cout << "Error in arguments of optimizer, check out in implementation." << std::endl;
//      return NULL;
//    }
//
//    int reinitializationInterval = -1;
//    if (timesReinitialization > 0)
//      reinitializationInterval = juce::jlimit(1, maxSteps, maxSteps / timesReinitialization);
//
//    // Init verbosity
//    String nameEnglobingScope("Monte carlo optimization : ");
//    int intervalVerbosity =
//        juce::jlimit(1, maxSteps, (int)std::ceil(maxSteps * frequencyVerbosity));
//    std::vector<Variable> resultCallbackValues;
//    if (verbosity)
//    {
//      std::vector<String> englobingScopesNames;
//      englobingScopesNames.push_back(nameEnglobingScope);
//      englobingScopesNames.push_back(T("Energies"));
//      englobingScopesNames.push_back(T("Energy"));
//      englobingScopesNames.push_back(T("Step"));
//      englobingScopesNames.push_back(T("Minimal energy"));
//      englobingScopesNames.push_back(T("Temporary energy"));
//      englobingScopesNames.push_back(T("Temperature"));
//      englobingScopesNames.push_back(T("Minimal energy (log10)"));
//      englobingScopesNames.push_back(T("Temporary energy (log10)"));
//      initializeCallbacks(context, englobingScopesNames, minimumEnergy);
//      resultCallbackValues.push_back(Variable((int)0));
//      resultCallbackValues.push_back(Variable(minimumEnergy));
//      resultCallbackValues.push_back(Variable(temporaryEnergy));
//      resultCallbackValues.push_back(Variable(temperature));
//      double logMinimumEnergy = minimumEnergy >= 1 ? log10(minimumEnergy) : -log10(std::abs(minimumEnergy - 2));
//      double logTemporaryEnergy = temporaryEnergy >= 1 ? log10(temporaryEnergy) : -log10(std::abs(temporaryEnergy - 2));
//      resultCallbackValues.push_back(Variable(logMinimumEnergy));
//      resultCallbackValues.push_back(Variable(logTemporaryEnergy));
//      callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
//    }
//    int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
//    String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
//    int indexOutputFile = 0;
//
//    if (saveToFile)
//    {
//      ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
//      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
//      File temporaryFile(temporaryOutputFileName);
//      protein->saveToXmlFile(context, temporaryFile);
//      indexOutputFile++;
//    }
//
//    for (int i = 1; i <= maxSteps; i++)
//    {
//      ProteinMoverPtr mover = sampler->sample(context, random).getObjectAndCast<ProteinMover>();
//      mover->move(workingPose);
//      temporaryEnergy = getConformationScore(workingPose, fullAtomEnergy);
//
//      if (keepConformation(random, temporaryEnergy - currentEnergy, temperature))
//      {
//        (*temporaryOptimizedPose) = (*workingPose);
//        currentEnergy = temporaryEnergy;
//      }
//      else
//      {
//        (*workingPose) = (*temporaryOptimizedPose);
//        temporaryEnergy = currentEnergy;
//      }
//
//      if (temporaryEnergy < minimumEnergy)
//      {
//        (*optimizedPose) = (*workingPose);
//        minimumEnergy = temporaryEnergy;
//      }
//
//      if ((reinitializationInterval > 0) && (i % reinitializationInterval) == 0)
//      {
//        (*workingPose) = (*optimizedPose);
//        (*temporaryOptimizedPose) = (*optimizedPose);
//        temporaryEnergy = minimumEnergy;
//        currentEnergy = minimumEnergy;
//      }
//
//      // Verbosity
//      if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
//      {
//        resultCallbackValues.at(0) = Variable((int)i);
//        resultCallbackValues.at(1) = Variable(minimumEnergy);
//        resultCallbackValues.at(2) = Variable(currentEnergy);
//        resultCallbackValues.at(3) = Variable(temperature);
//        resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
//        resultCallbackValues.at(5) = Variable(log10(temporaryEnergy));
//        callback(context, resultCallbackValues, Variable(minimumEnergy), maxSteps);
//      }
//
//      if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
//      {
//        ProteinPtr protein = convertPoseToProtein(context, optimizedPose);
//        String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
//        File temporaryFile(temporaryOutputFileName);
//        protein->saveToXmlFile(context, temporaryFile);
//        indexOutputFile++;
//      }
//    }
//    // Verbosity
//    if (verbosity)
//      finalizeCallbacks(context, minimumEnergy);
//
//    return optimizedPose;
  }
#endif // LBCPP_PROTEIN_ROSETTA

private:
  friend class ProteinMonteCarloOptimizerClass;

  double temperature;
  int maxSteps;
  int timesReinitialization;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_MONTE_CARLO_OPTIMIZER_H_
