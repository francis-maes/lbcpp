/*-----------------------------------------.---------------------------------.
| Filename: SimulatedAnnealing.h           | SimulatedAnnealing              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 1, 2012  8:21:03 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_

# include "GeneralOptimizer.h"

namespace lbcpp
{

class SimulatedAnnealingParameters : public GeneralOptimizerParameters
{
public:
  SimulatedAnnealingParameters() : maxIterations(0), initialTemperature(0), finalTemperature(0), numSteps(0) {}
  SimulatedAnnealingParameters(size_t maxIterations,
                               double initialTemperature,
                               double finalTemperature,
                               size_t numSteps)
    : maxIterations(maxIterations),
      initialTemperature(initialTemperature),
      finalTemperature(finalTemperature),
      numSteps(numSteps)
  {
    this->numSteps = juce::jmin((int)maxIterations, (int)numSteps);

    vector = variableVector(5);
    vector->setElement(1, Variable((int)maxIterations));
    vector->setElement(2, Variable(initialTemperature));
    vector->setElement(3, Variable(finalTemperature));
    vector->setElement(4, Variable((int)this->numSteps));
  }

  virtual VariableVectorPtr getParameters(ExecutionContext& context, const Variable& input) const
  {
    vector->setElement(0, Variable(getCurrentTemperature(input.getInteger())));
    return vector;
  }

protected:
  friend class SimulatedAnnealingParametersClass;

  virtual double getCurrentTemperature(size_t iteration) const
  {
    double x = std::floor((double)iteration * (double)numSteps / (double)maxIterations);
    return initialTemperature - x * ((initialTemperature - finalTemperature) / ((double)numSteps - 1));
  }

  VariableVectorPtr vector;
  size_t maxIterations;
  double initialTemperature;
  double finalTemperature;
  size_t numSteps;
};

typedef ReferenceCountedObjectPtr<SimulatedAnnealingParameters> SimulatedAnnealingParametersPtr;

class SimulatedAnnealing : public GeneralOptimizer
{
public:
  SimulatedAnnealing() {}
  SimulatedAnnealing(const OptimizationProblemStatePtr& initialState,
                     const OptimizationProblemStateModifierPtr& modifier,
                     const GeneralOptimizerStoppingCriterionPtr& stoppingCriterion,
                     const GeneralOptimizerParametersPtr& parameters)
    : GeneralOptimizer(initialState, modifier, stoppingCriterion, parameters) {}

  bool acceptanceCriterion(ExecutionContext& context, double temperature, double energyNewState, double energyCurrentState) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    if (energyNewState <= energyCurrentState)
      return true;
    else
      return random->sampleDouble() < std::exp((energyCurrentState - energyNewState) / temperature);
  }

  virtual VariableVectorPtr optimize(ExecutionContext& context)
  {
    // initialization
    jassert(parameters.isInstanceOf<SimulatedAnnealingParameters>());
    double verbosity = 0.01;
    DenseDoubleVectorPtr costEvolution = new DenseDoubleVector(0, 1.0);
    DenseDoubleVectorPtr acceptedModificationsEvolution = new DenseDoubleVector(0, 1.0);
    DenseDoubleVectorPtr decreasingModificationsEvolution = new DenseDoubleVector(0, 1.0);

    Variable iteration(0);
    VariableVectorPtr params = parameters->getParameters(context, iteration);
    maxIterations = (size_t)params->getElement(1).getInteger();
    double temperature = params->getElement(0).getDouble();

    double currentEnergy = initialState->getObjective(context);
    double newEnergy = 0.0;
    double bestEnergy = bestState->getObjective(context);
    OptimizationProblemStatePtr state = initialState;

    // acceptance ratio
    size_t numModificationsTested = 0;
    size_t numModificationsAccepted = 0;
    size_t numModificationsDecreasingEnergy = 0;
    context.enterScope(T("Simulated annealing"));

    for (size_t i = 0; i < maxIterations; i++)
    {
      // get new temperature
      iteration = Variable((int)i);
      params = parameters->getParameters(context, iteration);
      temperature = params->getElement(0).getDouble();

      // compute new state and its energy
      OptimizationProblemStatePtr newState = modifier->applyTo(context, state);
      newEnergy = newState->getObjective(context);
      numModificationsTested++;

      // accept or not the new state
      if (acceptanceCriterion(context, temperature, newEnergy, currentEnergy))
      {
        numModificationsAccepted++;
        if (newEnergy < currentEnergy)
          numModificationsDecreasingEnergy++;

        state = newState;
        currentEnergy = newEnergy;
      }

      // update the best state
      if (newEnergy < bestEnergy)
      {
        bestState = newState;
        bestEnergy = newEnergy;
      }

      if ((i == 0) || ((i + 1) == maxIterations) || ((i % (size_t)juce::jmax(1.0, verbosity * maxIterations)) == 0))
      {
        // verbosity in trace
        context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Interations")));
        context.enterScope(T("Iteration"));
        context.resultCallback(T("Iteration"), Variable((int)i));
        context.resultCallback(T("Current energy"), Variable(currentEnergy));
        context.resultCallback(T("Best energy"), Variable(bestEnergy));
        context.resultCallback(T("Temperature"), Variable(temperature));
        context.resultCallback(T("Ratio accepted modifications"), Variable((double)numModificationsAccepted / (double)numModificationsTested));
        context.resultCallback(T("Ratio energy decreasing modifications"), Variable((double)numModificationsDecreasingEnergy / (double)numModificationsTested));
        context.leaveScope(Variable(bestEnergy));

        // return values
        costEvolution->appendValue(bestEnergy);
        acceptedModificationsEvolution->appendValue((double)numModificationsAccepted / (double)numModificationsTested);
        decreasingModificationsEvolution->appendValue((double)numModificationsDecreasingEnergy / (double)numModificationsTested);

        // re initialization
        numModificationsTested = 0;
        numModificationsAccepted = 0;
        numModificationsDecreasingEnergy = 0;
      }

      if ((stoppingCriterion.get() != NULL) && (!stoppingCriterion->performNext(context, i, state, bestState)))
        break;
    }

    context.leaveScope();

    VariableVectorPtr returnVector = variableVector(3);
    returnVector->setElement(0, costEvolution);
    returnVector->setElement(1, acceptedModificationsEvolution);
    returnVector->setElement(2, decreasingModificationsEvolution);

    return returnVector;
  }

protected:
  friend class SimulatedAnnealingClass;

  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<SimulatedAnnealing> SimulatedAnnealingPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_
