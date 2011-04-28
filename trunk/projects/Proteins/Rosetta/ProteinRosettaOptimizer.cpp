/*-----------------------------------------.---------------------------------.
| Filename:  ProteinRosettaOptimizer.cpp   | ProteinRosettaOptimizer         |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinRosettaOptimizer.h"

using namespace lbcpp;

void RosettaOptimizer::initializeCallbacks(const std::vector<String>& names, double energy,
    int maxStepsNumber, ProteinMoverPtr& mover)
{
  if (verbosity)
  {
    nameScopesSet = true;
    nameScopes = names;
    String nameScope(nameScopes.at(0));
    nameScope += name;
    context->enterScope(nameScope);
    context->resultCallback(T("Protein name"), name);
    if (mover.get() != NULL)
    {
      context->resultCallback(T("Mover Name"), mover->getName());
      std::vector<String> names = mover->getParametersNames();
      std::vector<Variable> parameters = mover->getParameters();
      String templateName("Mover parameter : ");
      for (int i = 0; i < names.size(); i++)
        context->resultCallback(templateName + names.at(i), parameters.at(i));
    }
    context->resultCallback(T("Initial Energy"), Variable(energy));
    context->enterScope(nameScopes.at(1));
  }
}

void RosettaOptimizer::finalizeCallbacks(double energy)
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

void RosettaOptimizer::callback(const std::vector<Variable>& resultCallbackValues,
    Variable returnValue, int maxStepsNumber)
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

void RosettaOptimizer::setVerbosity(bool verbosity)
{
  this->verbosity = verbosity;
}

bool RosettaOptimizer::getVerbosity()
{
  return verbosity;
}

void RosettaOptimizer::setFrequency(double frequencyCallback)
{
  frequencyVerbosity = frequencyCallback;
}

double RosettaOptimizer::getFrequency()
{
  return frequencyVerbosity;
}

RosettaOptimizer::RosettaOptimizer(long long seedForRandom)
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

RosettaOptimizer::RosettaOptimizer(ExecutionContextPtr context, String name,
    double frequencyCallback, File outputDirectory, int numOutputFiles, long long seedForRandom)
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

String RosettaOptimizer::getProteinName()
{
  return String(name);
}

void RosettaOptimizer::setProteinName(String name)
{
  this->name = name;
}

bool RosettaOptimizer::keepConformation(double deltaEnergy, double temperature)
{
  double val = std::exp(-deltaEnergy / temperature);

  if (randomGenerator->sampleDouble() < val)
    return true;
  else
    return false;
}

// Greedy optimizer
RosettaGreedyOptimizer::RosettaGreedyOptimizer(long long seedForRandom) :
  RosettaOptimizer(seedForRandom), maxSteps(50000)
{
}

RosettaGreedyOptimizer::RosettaGreedyOptimizer(ExecutionContextPtr context, String name,
    double frequencyCallback, File outputDirectory, int numOutputFiles, long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), maxSteps(50000)
{
}

RosettaGreedyOptimizer::RosettaGreedyOptimizer(int maxSteps, long long seedForRandom) :
  RosettaOptimizer(seedForRandom), maxSteps(maxSteps)
{
}

RosettaGreedyOptimizer::RosettaGreedyOptimizer(int maxSteps, ExecutionContextPtr context,
    String name, double frequencyCallback, File outputDirectory, int numOutputFiles,
    long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), maxSteps(maxSteps)
{
}

core::pose::PoseOP RosettaGreedyOptimizer::apply(core::pose::PoseOP& pose, ProteinMoverPtr& mover)
{
  return greedyOptimization(pose, mover, maxSteps);
}

core::pose::PoseOP RosettaGreedyOptimizer::greedyOptimization(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover, int maxSteps)
{
  // Initialization
  double minimumEnergy = getConformationScore(pose);
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
  int intervalVerbosity = juce::jlimit(1, maxSteps, (int)std::ceil(maxSteps * frequencyVerbosity));
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, maxSteps, mover);
    resultCallbackValues.push_back(Variable((int)0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
  }
  int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
  String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
  int indexOutputFile = 0;

  if (saveToFile)
  {
    ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
    String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
    File temporaryFile(temporaryOutputFileName);
    protein->saveToXmlFile(*context, temporaryFile);
    indexOutputFile++;
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getConformationScore(workingPose);

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
      callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
    }

    if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
    {
      ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
      File temporaryFile(temporaryOutputFileName);
      protein->saveToXmlFile(*context, temporaryFile);
      indexOutputFile++;
    }
  }

  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  // Return
  return optimizedPose;
}

// MonteCarlo optimizer
RosettaMonteCarloOptimizer::RosettaMonteCarloOptimizer(long long seedForRandom) :
  RosettaOptimizer(seedForRandom), temperature(1.0), maxSteps(50000), timesReinitialization(5)
{
}

RosettaMonteCarloOptimizer::RosettaMonteCarloOptimizer(ExecutionContextPtr context, String name,
    double frequencyCallback, File outputDirectory, int numOutputFiles, long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), temperature(1.0), maxSteps(50000), timesReinitialization(5)
{
}

RosettaMonteCarloOptimizer::RosettaMonteCarloOptimizer(double temperature, int maxSteps,
    int timesReinitialization, long long seedForRandom) :
  RosettaOptimizer(seedForRandom), temperature(temperature), maxSteps(maxSteps),
      timesReinitialization(timesReinitialization)
{
}

RosettaMonteCarloOptimizer::RosettaMonteCarloOptimizer(double temperature, int maxSteps,
    int timesReinitialization, ExecutionContextPtr context, String name, double frequencyCallback,
    File outputDirectory, int numOutputFiles, long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), temperature(temperature), maxSteps(maxSteps), timesReinitialization(
          timesReinitialization)
{
}

core::pose::PoseOP RosettaMonteCarloOptimizer::apply(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover)
{
  return monteCarloOptimization(pose, mover, temperature, maxSteps, timesReinitialization);
}

core::pose::PoseOP RosettaMonteCarloOptimizer::monteCarloOptimization(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover, double temperature, int maxSteps, int timesReinitialization)
{
  double currentEnergy = getConformationScore(pose);
  double minimumEnergy = currentEnergy;
  double temporaryEnergy = currentEnergy;
  core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

  if ((maxSteps <= 0) || (temperature <= 0))
  {
    std::cout << "Error in arguments of optimizer, check out in implementation." << std::endl;
    return NULL;
  }

  int reinitializationInterval = -1;
  if (timesReinitialization > 0)
    reinitializationInterval = juce::jlimit(1, maxSteps, maxSteps / timesReinitialization);

  // Init verbosity
  String nameEnglobingScope("Monte carlo optimization : ");
  int intervalVerbosity = juce::jlimit(1, maxSteps, (int)std::ceil(maxSteps * frequencyVerbosity));
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, maxSteps, mover);
    resultCallbackValues.push_back(Variable((int)0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(temperature));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
  }
  int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
  String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
  int indexOutputFile = 0;

  if (saveToFile)
  {
    ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
    String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
    File temporaryFile(temporaryOutputFileName);
    protein->saveToXmlFile(*context, temporaryFile);
    indexOutputFile++;
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getConformationScore(workingPose);

    if (keepConformation(temporaryEnergy - currentEnergy, temperature))
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

    if ((reinitializationInterval > 0) && (i % reinitializationInterval) == 0)
    {
      (*workingPose) = (*optimizedPose);
      (*temporaryOptimizedPose) = (*optimizedPose);
      temporaryEnergy = minimumEnergy;
      currentEnergy = minimumEnergy;
    }

    // Verbosity
    if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
    {
      resultCallbackValues.at(0) = Variable((int)i);
      resultCallbackValues.at(1) = Variable(minimumEnergy);
      resultCallbackValues.at(2) = Variable(currentEnergy);
      resultCallbackValues.at(3) = Variable(temperature);
      resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
      resultCallbackValues.at(5) = Variable(log10(temporaryEnergy));
      callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
    }

    if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
    {
      ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
      File temporaryFile(temporaryOutputFileName);
      protein->saveToXmlFile(*context, temporaryFile);
      indexOutputFile++;
    }
  }
  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  return optimizedPose;
}

// Simulated annealing optimizer
RosettaSimulatedAnnealingOptimizer::RosettaSimulatedAnnealingOptimizer(long long seedForRandom) :
  RosettaOptimizer(seedForRandom), initialTemperature(4.0), finalTemperature(0.01),
      numberDecreasingSteps(100), maxSteps(50000), timesReinitialization(5)
{
}

RosettaSimulatedAnnealingOptimizer::RosettaSimulatedAnnealingOptimizer(ExecutionContextPtr context,
    String name, double frequencyCallback, File outputDirectory, int numOutputFiles,
    long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), initialTemperature(4.0), finalTemperature(0.01), numberDecreasingSteps(
          100), maxSteps(50000), timesReinitialization(5)
{
}

RosettaSimulatedAnnealingOptimizer::RosettaSimulatedAnnealingOptimizer(double initialTemperature,
    double finalTemperature, int numberDecreasingSteps, int maxSteps, int timesReinitialization,
    long long seedForRandom) :
  RosettaOptimizer(seedForRandom), initialTemperature(initialTemperature), finalTemperature(
      finalTemperature), numberDecreasingSteps(numberDecreasingSteps), maxSteps(maxSteps),
      timesReinitialization(timesReinitialization)
{
}

RosettaSimulatedAnnealingOptimizer::RosettaSimulatedAnnealingOptimizer(double initialTemperature,
    double finalTemperature, int numberDecreasingSteps, int maxSteps, int timesReinitialization,
    ExecutionContextPtr context, String name, double frequencyCallback, File outputDirectory,
    int numOutputFiles, long long seedForRandom) :
      RosettaOptimizer(context, name, frequencyCallback, outputDirectory, numOutputFiles,
          seedForRandom), initialTemperature(initialTemperature),
      finalTemperature(finalTemperature), numberDecreasingSteps(numberDecreasingSteps), maxSteps(
          maxSteps), timesReinitialization(timesReinitialization)
{
}

core::pose::PoseOP RosettaSimulatedAnnealingOptimizer::apply(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover)
{
  return simulatedAnnealingOptimization(pose, mover, initialTemperature, finalTemperature,
      numberDecreasingSteps, maxSteps, timesReinitialization);
}

core::pose::PoseOP RosettaSimulatedAnnealingOptimizer::simulatedAnnealingOptimization(
    core::pose::PoseOP& pose, ProteinMoverPtr& mover, double initialTemperature,
    double finalTemperature, int numberDecreasingSteps, int maxSteps, int timesReinitialization)
{
  double currentEnergy = getConformationScore(pose);
  double minimumEnergy = currentEnergy;
  double temporaryEnergy = currentEnergy;

  if ((initialTemperature < finalTemperature) || (numberDecreasingSteps > maxSteps)
      || (numberDecreasingSteps <= 0) || (maxSteps <= 0) || (initialTemperature <= 0)
      || (finalTemperature <= 0))
  {
    std::cout << "Error in arguments of optimizer, check out in implementation." << std::endl;
    return NULL;
  }

  int reinitializationInterval = -1;
  if (timesReinitialization > 0)
    reinitializationInterval = juce::jlimit(1, maxSteps, maxSteps / timesReinitialization);

  double currentTemperature = initialTemperature;
  int intervalDecreasingTemperature = maxSteps / numberDecreasingSteps;
  core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

  // Init verbosity
  String nameEnglobingScope("Simulated annealing optimization : ");
  int intervalVerbosity = juce::jlimit(1, maxSteps, (int)std::ceil(maxSteps * frequencyVerbosity));
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, maxSteps, mover);
    resultCallbackValues.push_back(Variable((int)0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(currentTemperature));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
  }
  int intervalSaveToFile = juce::jlimit(1, maxSteps, maxSteps / numOutputFiles);
  String nameOutputFile = outputDirectory.getFullPathName() + T("/") + name + T("_");
  int indexOutputFile = 0;

  if (saveToFile)
  {
    ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
    String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
    File temporaryFile(temporaryOutputFileName);
    protein->saveToXmlFile(*context, temporaryFile);
    indexOutputFile++;
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getConformationScore(workingPose);

    if (keepConformation(temporaryEnergy - currentEnergy, currentTemperature))
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

    if ((reinitializationInterval > 0) && (i % reinitializationInterval) == 0)
    {
      (*workingPose) = (*optimizedPose);
      (*temporaryOptimizedPose) = (*optimizedPose);
      temporaryEnergy = minimumEnergy;
      currentEnergy = minimumEnergy;
    }

    if ((i % intervalDecreasingTemperature) == 0)
    {
      currentTemperature = currentTemperature - ((initialTemperature - finalTemperature)
          / (double)numberDecreasingSteps);
    }

    // Verbosity
    if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
    {
      resultCallbackValues.at(0) = Variable((int)i);
      resultCallbackValues.at(1) = Variable(minimumEnergy);
      resultCallbackValues.at(2) = Variable(currentEnergy);
      resultCallbackValues.at(3) = Variable(currentTemperature);
      resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
      resultCallbackValues.at(5) = Variable(log10(currentEnergy));
      callback(resultCallbackValues, Variable(minimumEnergy), maxSteps);
    }

    if (saveToFile && (((i % intervalSaveToFile) == 0) || (i == maxSteps)))
    {
      ProteinPtr protein = convertPoseToProtein(*context, optimizedPose);
      String temporaryOutputFileName = nameOutputFile + String(indexOutputFile) + T(".xml");
      File temporaryFile(temporaryOutputFileName);
      protein->saveToXmlFile(*context, temporaryFile);
      indexOutputFile++;
    }
  }
  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  return optimizedPose;
}

// Seqential optimizer
RosettaSequentialOptimizer::RosettaSequentialOptimizer(long long seedForRandom) :
  RosettaOptimizer(seedForRandom)
{
}

RosettaSequentialOptimizer::RosettaSequentialOptimizer(ExecutionContextPtr context, String name,
    double frequencyCallback, long long seedForRandom) :
  RosettaOptimizer(context, name, frequencyCallback, juce::File(), -1, seedForRandom)
{
}

core::pose::PoseOP RosettaSequentialOptimizer::apply(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover)
{
  return sequentialOptimization(pose, mover);
}

core::pose::PoseOP RosettaSequentialOptimizer::sequentialOptimization(core::pose::PoseOP& pose,
    ProteinMoverPtr& mover)
{
  core::pose::PoseOP acc = new core::pose::Pose();

  double initialTemperature = 4.0;
  double finalTemperature = 0.01;
  int numberDecreasingSteps = 100;
  int maxSteps = 0;
  int factor = 5000;
  bool store = getVerbosity();
  setVerbosity(false);

  for (int i = 1; i <= pose->n_residue(); i++)
  {
    RosettaSimulatedAnnealingOptimizerPtr optimizer = new RosettaSimulatedAnnealingOptimizer();

    maxSteps = (int)std::max((int)((factor * log(i)) + 1), numberDecreasingSteps);
    acc->append_residue_by_bond(pose->residue(i), true);
    core::pose::PoseOP tempPose = optimizer-> simulatedAnnealingOptimization(acc, mover,
        initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps);
    //core::pose::PoseOP tempPose = monteCarloOptimization(acc, mover, optArgs, finalTemperature,
    //    maxSteps);
    if (tempPose.get() == NULL)
    {
      return NULL;
    }
    (*acc) = (*tempPose);
  }

  setVerbosity(store);
  return acc;
}
