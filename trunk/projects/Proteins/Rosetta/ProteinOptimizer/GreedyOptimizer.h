/*-----------------------------------------.---------------------------------.
| Filename: GreedyOptimizer.h              | ProteinGreedyOptimizer          |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  22:44:00         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_GREEDY_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_GREEDY_OPTIMIZER_H_

# include "precompiled.h"
# include "../ProteinOptimizer.h"
# include "../Sampler.h"
# include "../ProteinMover.h"

namespace lbcpp
{
class ProteinGreedyOptimizer;
typedef ReferenceCountedObjectPtr<ProteinGreedyOptimizer> ProteinGreedyOptimizerPtr;

class ProteinGreedyOptimizer: public ProteinOptimizer
{
public:
  ProteinGreedyOptimizer() :
    ProteinOptimizer()
  {
  }

  ProteinGreedyOptimizer(int maxSteps, String name = juce::String("Default"),
      double frequencyCallback = 0.01, int numOutputFiles = -1, File outputDirectory = juce::File()) :
    ProteinOptimizer(name, frequencyCallback, numOutputFiles, outputDirectory), maxSteps(maxSteps)
  {
  }

  virtual void apply(ExecutionContext& context, const core::pose::PoseOP& pose,
                     const RandomGeneratorPtr& random, const SamplerPtr& sampler, core::pose::PoseOP& res)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    return greedyOptimization(pose, sampler, context, random, maxSteps);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

#ifdef LBCPP_PROTEIN_ROSETTA
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
  core::pose::PoseOP greedyOptimization(const core::pose::PoseOP& pose, const SamplerPtr& sampler,
      ExecutionContext& context, const RandomGeneratorPtr& random, int maxSteps = 50000)
  {
    // Initialization
    double minimumEnergy = getConformationScore(pose, fullAtomEnergy);
    double temporaryEnergy = minimumEnergy;
    core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
    core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

    if (maxSteps <= 0)
    {
      std::cout << "Error in arguments of optimizer, check out in implementation." << std::endl;
      return NULL;
    }

    // Init verbosity
    String nameEnglobingScope("Greedy optimization : ");
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
      englobingScopesNames.push_back(T("Minimal energy (log10)"));
      englobingScopesNames.push_back(T("Temporary energy (log10)"));
      initializeCallbacks(context, englobingScopesNames, minimumEnergy);
      resultCallbackValues.push_back(Variable((int)0));
      resultCallbackValues.push_back(Variable(minimumEnergy));
      resultCallbackValues.push_back(Variable(temporaryEnergy));
      resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
      resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
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
      ProteinMoverPtr mover = sampler->sample(context, random).getObjectAndCast<ProteinMover> ();
      mover->move(workingPose);
      temporaryEnergy = getConformationScore(workingPose, fullAtomEnergy);

      if (temporaryEnergy < minimumEnergy)
      {
        (*optimizedPose) = (*workingPose);
        minimumEnergy = temporaryEnergy;
      }
      else
      {
        (*workingPose) = (*optimizedPose);
      }

      // Verbosity
      if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
      {
        resultCallbackValues.at(0) = Variable((int)i);
        resultCallbackValues.at(1) = Variable(minimumEnergy);
        resultCallbackValues.at(2) = Variable(temporaryEnergy);
        resultCallbackValues.at(3) = Variable(log10(minimumEnergy));
        resultCallbackValues.at(4) = Variable(log10(temporaryEnergy));
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

    // Return
    return optimizedPose;
  }
#endif // LBCPP_PROTEIN_ROSETTA

private:
  friend class ProteinGreedyOptimizerClass;
  int maxSteps;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_GREEDY_OPTIMIZER_H_
