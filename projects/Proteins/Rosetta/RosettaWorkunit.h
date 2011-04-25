/*-----------------------------------------.---------------------------------.
| Filename: RosettaWorkunit.h              | Rosetta Workunits               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 22/04/2011 15:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_

# include "precompiled.h"
# include "../Data/Protein.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "RosettaUtils.h"
# include "RosettaOptimizer.h"

namespace lbcpp
{

class RosettaProteinOptimizerAndFeatureGeneratorWorkUnit;
typedef ReferenceCountedObjectPtr<RosettaProteinOptimizerAndFeatureGeneratorWorkUnit>
    RosettaProteinOptimizerAndFeatureGeneratorWorkUnitPtr;
class RosettaProteinOptimizerAndFeatureGeneratorWorkUnit: public WorkUnit
{
protected:
  friend class RosettaProteinOptimizerAndFeatureGeneratorWorkUnitClass;
  String proteinName;
  core::pose::PoseOP pose;
  RosettaOptimizerPtr optimizer;
  RosettaMoverPtr mover;
  core::pose::PoseOP returnPose;

public:
  // returnPose must be already instanciated
  RosettaProteinOptimizerAndFeatureGeneratorWorkUnit()
  {
  }

  RosettaProteinOptimizerAndFeatureGeneratorWorkUnit(const String& proteinName, core::pose::PoseOP& pose,
      RosettaOptimizerPtr& optimizer, RosettaMoverPtr& mover, core::pose::PoseOP& returnPose)
  {
    this->proteinName = proteinName;
    this->pose = pose;
    this->optimizer = optimizer;
    this->mover = mover;
    this->returnPose = returnPose;
  }

  virtual Variable run(ExecutionContext& context)
  {
    context.informationCallback(T("Protein : ") + proteinName + T(" loaded succesfully."));
    context.resultCallback(T("Initial energy"), Variable(getConformationScore(pose)));
    *returnPose = *pose;
    core::pose::PoseOP temporaryPose = new core::pose::Pose((*pose));
    *returnPose = (*(optimizer->apply(temporaryPose, mover)));
    context.informationCallback(T("Optimization done."));
    double score = getConformationScore(returnPose);
    context.resultCallback(T("Final energy"), Variable(score));

    return Variable(proteinName);
  }
};

class RosettaProteinsFeaturesGeneratorWorkUnit: public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);

    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
      context.errorCallback(T("Proteins' directory not found."));
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, directory);
    int numProteins = proteins->getNumElements();

    // Mover
    std::vector<Variable> moverArguments;
    for (int i = 0; i < commandLineMoverParameters.size(); i++)
      moverArguments.push_back(Variable(commandLineMoverParameters[i]));
    RosettaMoverPtr mover;
    if (!moverName.compareIgnoreCase(T("phipsirandom")))
      mover = new PhiPsiRandomMover(moverArguments);
    else if (!moverName.compareIgnoreCase(T("phipsigaussrandom")))
      mover = new PhiPsiGaussRandomMover(moverArguments);
    else if (!moverName.compareIgnoreCase(T("shearrandom")))
      mover = new ShearRandomMover(moverArguments);
    else if (!moverName.compareIgnoreCase(T("rigidbodytransrandom")))
      mover = new RigidBodyTransRandomMover(moverArguments);
    else if (!moverName.compareIgnoreCase(T("rigidbodyperturbrandom")))
      mover = new RigidBodyPerturbRandomMover(moverArguments);
    else
    {
      context.errorCallback(T("Bad mover name."));
      return Variable();
    }

    // Other arguments
    juce::File outputDirectory = context.getFile(resultsDirectory);
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    double frequenceVerbosity = 0.01;

    // Creating parallel workunits
    CompositeWorkUnitPtr proteinsOptimizer = new CompositeWorkUnit(T("ProteinsOptimizer"),
        numProteins);
    for (int i = 0; i < proteinsOptimizer->getNumWorkUnits(); i++)
    {
      ProteinPtr currentProtein = proteins->getElement(i).getObjectAndCast<Protein>();
      core::pose::PoseOP currentPose = convertProteinToPose(context, currentProtein);
      core::pose::PoseOP returnPose = new core::pose::Pose();

      // Optimizer
      RosettaOptimizerPtr optimizer;
      if (!optimizerName.compareIgnoreCase(T("greedy")))
        optimizer = new RosettaGreedyOptimizer(maxNumberIterations, &context,
            currentProtein->getName(), frequenceVerbosity, outputDirectory, timesFeatureGeneration);
      else if (!optimizerName.compareIgnoreCase(T("montecarlo")))
        optimizer = new RosettaMonteCarloOptimizer(commandLineOptimizerParameters[0],
            maxNumberIterations, (int)commandLineOptimizerParameters[1], &context,
            currentProtein->getName(), frequenceVerbosity, outputDirectory, timesFeatureGeneration);
      else if (!optimizerName.compareIgnoreCase(T("simulatedannealing")))
        optimizer = new RosettaSimulatedAnnealingOptimizer(commandLineOptimizerParameters[0],
            commandLineOptimizerParameters[1], (int)commandLineOptimizerParameters[2],
            maxNumberIterations, (int)commandLineOptimizerParameters[3], &context,
            currentProtein->getName(), frequenceVerbosity, outputDirectory, timesFeatureGeneration);
      else if (!optimizerName.compareIgnoreCase(T("sequential")))
        optimizer = new RosettaSequentialOptimizer(&context, currentProtein->getName(),
            frequenceVerbosity);
      else
      {
        context.errorCallback(T("Bad optimizer name."));
        return Variable();
      }

      WorkUnitPtr childWorkUnit = new RosettaProteinOptimizerAndFeatureGeneratorWorkUnit(
          currentProtein->getName(), currentPose, optimizer, mover, returnPose);
      proteinsOptimizer->setWorkUnit(i, childWorkUnit);
    }
    //proteinsOptimizer->setPushChildrenIntoStackFlag(true);
    proteinsOptimizer->setPushChildrenIntoStackFlag(false);
    context.informationCallback(T("Computing..."));
    context.run(proteinsOptimizer);

    context.informationCallback(T("RosettaProteinsFeaturesGeneratorWorkUnit done."));
    return Variable();
  }

protected:
  friend class RosettaProteinsFeaturesGeneratorWorkUnitClass;
  String proteinsDirectory;
  String resultsDirectory;
  String moverName;
  std::vector<double> commandLineMoverParameters;

  int timesFeatureGeneration;
  String optimizerName;
  int maxNumberIterations;
  std::vector<double> commandLineOptimizerParameters;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
