/*-----------------------------------------.---------------------------------.
| Filename:  RosettaOptimizer.cpp          | RosettaOptimizer                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "RosettaOptimizer.h"

using namespace lbcpp;

void RosettaOptimizer::initializeCallbacks(const std::vector<String>& names, double energy,
    int maxStepsNumber, RosettaMoverPtr mover)
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
      context->progressCallback(new ProgressionState((double)0, (double)maxStepsNumber,
          T("Iterations")));
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

void RosettaOptimizer::callback(const std::vector<Variable>& resultCallbackValues, Variable returnValue,
    int maxStepsNumber)
{
  if (verbosity)
  {
    if (nameScopesSet)
    {
      context->progressCallback(new ProgressionState((double)resultCallbackValues.at(0).getInteger(),
          (double)maxStepsNumber, T("Iterations")));
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

void RosettaOptimizer::setFrequency(double frequency)
{
  frequencyVerbosity = frequency;
}

double RosettaOptimizer::getFrequency()
{
  return frequencyVerbosity;
}

RosettaOptimizer::RosettaOptimizer()
{
  context = NULL;
  name = (String) "Default";
  frequencyVerbosity = 0.1;
  verbosity = false;
  nameScopesSet = false;
}

RosettaOptimizer::RosettaOptimizer(ExecutionContextPtr context, String name, double frequency)
{
  if (context.get() != NULL)
  {
    this->context = context;
    if (name.isEmpty())
      this->name = (String) "Default";
    else
      this->name = name;
    if ((frequency < 0) || (frequency > 1))
      frequencyVerbosity = 0.1;
    else
      frequencyVerbosity = frequency;
    verbosity = true;
  }
  else
  {
    this->context = NULL;
    this->name = (String) "Default";
    frequencyVerbosity = 0.1;
    verbosity = false;
  }
  nameScopesSet = false;
}

RosettaOptimizer::~RosettaOptimizer()
{
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

  if (generateRand() < val)
    return true;
  else
    return false;
}

core::pose::PoseOP RosettaOptimizer::monteCarloOptimization(core::pose::PoseOP pose,
    RosettaMoverPtr mover, double temperature, int maxSteps, int timesReinitialization)
{
  double currentEnergy = getTotalEnergy(pose);
  double minimumEnergy = currentEnergy;
  double temporaryEnergy = currentEnergy;
  core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

  int reinitializationInterval = -1;
  if (timesReinitialization > 0)
    reinitializationInterval = maxSteps / timesReinitialization;

  if ((maxSteps <= 0) || (temperature <= 0))
    return NULL;

  // Init verbosity
  String nameEnglobingScope("Monte carlo optimization : ");
  int intervalVerbosity = (int) std::ceil(maxSteps * frequencyVerbosity);
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, mover);
    resultCallbackValues.push_back(Variable((int) 0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(temperature));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), 0.0);
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getTotalEnergy(workingPose);

    if (keepConformation(temporaryEnergy - currentEnergy, temperature))
    {
      temporaryOptimizedPose->set_new_conformation(&(workingPose->conformation()));
      currentEnergy = temporaryEnergy;
    }
    else
    {
      workingPose->set_new_conformation(&(temporaryOptimizedPose->conformation()));
      temporaryEnergy = currentEnergy;
    }

    if (temporaryEnergy < minimumEnergy)
    {
      optimizedPose->set_new_conformation(&(workingPose->conformation()));
      minimumEnergy = temporaryEnergy;
    }

    if ((reinitializationInterval > 0) && (i % reinitializationInterval) == 0)
    {
      workingPose->set_new_conformation(&(optimizedPose->conformation()));
      temporaryOptimizedPose->set_new_conformation(&(optimizedPose->conformation()));
      temporaryEnergy = minimumEnergy;
      currentEnergy = minimumEnergy;
    }

    // Verbosity
    if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
    {
      resultCallbackValues.at(0) = Variable((int) i);
      resultCallbackValues.at(1) = Variable(minimumEnergy);
      resultCallbackValues.at(2) = Variable(currentEnergy);
      resultCallbackValues.at(3) = Variable(temperature);
      resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
      resultCallbackValues.at(5) = Variable(log10(temporaryEnergy));
      callback(resultCallbackValues, Variable(minimumEnergy), (double)i / (double)maxSteps);
    }
  }
  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  return optimizedPose;
}

core::pose::PoseOP RosettaOptimizer::simulatedAnnealingOptimization(core::pose::PoseOP pose,
    RosettaMoverPtr mover, double initialTemperature, double finalTemperature,
    int numberDecreasingSteps, int maxSteps, int timesReinitialization)
{
  double currentEnergy = getTotalEnergy(pose);
  double minimumEnergy = currentEnergy;
  double temporaryEnergy = currentEnergy;
  if ((initialTemperature < finalTemperature) || (numberDecreasingSteps > maxSteps) || (numberDecreasingSteps <= 0) || (maxSteps <= 0)
      || (initialTemperature <= 0) || (finalTemperature <= 0))
    return NULL;

  int reinitializationInterval = -1;
  if (timesReinitialization > 0)
    reinitializationInterval = maxSteps / timesReinitialization;

  double currentTemperature = initialTemperature;
  int intervalDecreasingTemperature = maxSteps / numberDecreasingSteps;
  core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP temporaryOptimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

  // Init verbosity
  String nameEnglobingScope("Simulated annealing optimization : ");
  int intervalVerbosity = (int) std::ceil(maxSteps * frequencyVerbosity);
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, mover);
    resultCallbackValues.push_back(Variable((int) 0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(currentTemperature));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), 0.0);
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getTotalEnergy(workingPose);

    if (keepConformation(temporaryEnergy - currentEnergy, currentTemperature))
    {
      temporaryOptimizedPose->set_new_conformation(&(workingPose->conformation()));
      currentEnergy = temporaryEnergy;
    }
    else
    {
      workingPose->set_new_conformation(&(temporaryOptimizedPose->conformation()));
      temporaryEnergy = currentEnergy;
    }

    if (temporaryEnergy < minimumEnergy)
    {
      optimizedPose->set_new_conformation(&(workingPose->conformation()));
      minimumEnergy = temporaryEnergy;
    }

    if ((reinitializationInterval > 0) && (i % reinitializationInterval) == 0)
    {
      workingPose->set_new_conformation(&(optimizedPose->conformation()));
      temporaryOptimizedPose->set_new_conformation(&(optimizedPose->conformation()));
      temporaryEnergy = minimumEnergy;
      currentEnergy = minimumEnergy;
    }

    if ((i % intervalDecreasingTemperature) == 0)
    {
      currentTemperature = currentTemperature - ((initialTemperature - finalTemperature) / (double) numberDecreasingSteps);
    }

    // Verbosity
    if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
    {
      resultCallbackValues.at(0) = Variable((int) i);
      resultCallbackValues.at(1) = Variable(minimumEnergy);
      resultCallbackValues.at(2) = Variable(currentEnergy);
      resultCallbackValues.at(3) = Variable(currentTemperature);
      resultCallbackValues.at(4) = Variable(log10(minimumEnergy));
      resultCallbackValues.at(5) = Variable(log10(currentEnergy));
      callback(resultCallbackValues, Variable(minimumEnergy), (double)i / (double)maxSteps);
    }
  }
  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  return optimizedPose;
}

core::pose::PoseOP RosettaOptimizer::greedyOptimization(core::pose::PoseOP pose,
    RosettaMoverPtr mover, int maxSteps)
{
  // Initialization
  double minimumEnergy = getTotalEnergy(pose);
  double temporaryEnergy = minimumEnergy;
  core::pose::PoseOP optimizedPose = new core::pose::Pose((*pose));
  core::pose::PoseOP workingPose = new core::pose::Pose((*pose));

  if (maxSteps <= 0)
    return NULL;

  // Init verbosity
  String nameEnglobingScope("Greedy optimization : ");
  int intervalVerbosity = (int) std::ceil(maxSteps * frequencyVerbosity);
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
    initializeCallbacks(englobingScopesNames, minimumEnergy, mover);
    resultCallbackValues.push_back(Variable((int) 0));
    resultCallbackValues.push_back(Variable(minimumEnergy));
    resultCallbackValues.push_back(Variable(temporaryEnergy));
    resultCallbackValues.push_back(Variable(log10(minimumEnergy)));
    resultCallbackValues.push_back(Variable(log10(temporaryEnergy)));
    callback(resultCallbackValues, Variable(minimumEnergy), 0.0);
  }

  for (int i = 1; i <= maxSteps; i++)
  {
    mover->move(workingPose);
    temporaryEnergy = getTotalEnergy(workingPose);

    if (temporaryEnergy < minimumEnergy)
    {
      optimizedPose->set_new_conformation(&(workingPose->conformation()));
      minimumEnergy = temporaryEnergy;
    }
    else
    {
      workingPose->set_new_conformation(&(optimizedPose->conformation()));
    }

    // Verbosity
    if (verbosity && (((i % intervalVerbosity) == 0) || (i == maxSteps)))
    {
      resultCallbackValues.at(0) = Variable((int) i);
      resultCallbackValues.at(1) = Variable(minimumEnergy);
      resultCallbackValues.at(2) = Variable(temporaryEnergy);
      resultCallbackValues.at(3) = Variable(log10(minimumEnergy));
      resultCallbackValues.at(4) = Variable(log10(temporaryEnergy));
      callback(resultCallbackValues, Variable(minimumEnergy), (double)i / (double)maxSteps);
    }
  }

  // Verbosity
  if (verbosity)
    finalizeCallbacks(minimumEnergy);

  // Return
  return optimizedPose;
}

core::pose::PoseOP RosettaOptimizer::sequentialOptimization(core::pose::PoseOP pose,
    RosettaMoverPtr mover)
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
    maxSteps = (int) std::max((int) ((factor * log(i)) + 1), numberDecreasingSteps);
    acc->append_residue_by_bond(pose->residue(i), true);
    core::pose::PoseOP tempPose = simulatedAnnealingOptimization(acc, mover, initialTemperature, finalTemperature,
        numberDecreasingSteps, maxSteps);
    //core::pose::PoseOP tempPose = monteCarloOptimization(acc, mover, optArgs, finalTemperature,
    //		maxSteps);
    if (tempPose.get() == NULL)
    {
      return NULL;
    }
    acc->set_new_conformation(&(tempPose->conformation()));
  }

  setVerbosity(store);
  return acc;
}

