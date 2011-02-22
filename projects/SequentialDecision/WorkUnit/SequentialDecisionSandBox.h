/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionSandBox.h    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

# include "../Problem/LinearPointPhysicProblem.h"
# include "../Core/SearchSpace.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class EvaluateSearchHeuristicWorkUnit : public WorkUnit
{
public:
  EvaluateSearchHeuristicWorkUnit(SequentialDecisionProblemPtr problem, const Variable& initialState, SearchHeuristicPtr heuristic, size_t maxSearchNodes, double discount, double& result)
    : problem(problem), initialState(initialState), heuristic(heuristic), maxSearchNodes(maxSearchNodes), discount(discount), result(result) {}
  EvaluateSearchHeuristicWorkUnit() : result(*(double* )0) {}

  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    SortedSearchSpacePtr searchSpace = new SortedSearchSpace(problem, heuristic, discount, initialState);
    searchSpace->reserveNodes(2 * maxSearchNodes);

    double highestReturn = 0.0;
    for (size_t j = 0; j < maxSearchNodes; ++j)
    {
      double value = searchSpace->exploreBestNode(context);
      if (value > highestReturn)
        highestReturn = value;
    }

    return result = highestReturn;
  }

protected:
  SequentialDecisionProblemPtr problem;
  Variable initialState;
  SearchHeuristicPtr heuristic;
  size_t maxSearchNodes;
  double discount;
  double& result;
};

class SequentialDecisionSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    SequentialDecisionProblemPtr system = linearPointPhysicSystem();
    if (!system->initialize(context))
      return false;
    
    static const double discountFactor = 0.9;
    static const size_t numSamples = 1000;

    for (size_t d = 2; d <= 18; ++d)
    {
      context.enterScope(T("Computing scores for depth = ") + String((int)d));
      context.resultCallback(T("depth"), d);

      size_t maxSearchNodes = (size_t)pow(2.0, (double)(d + 1)) - 1;
      context.resultCallback(T("maxSearchNodes"), maxSearchNodes);
      
      double minDepthScore = evaluateSearchHeuristic(context, system, minDepthSearchHeuristic(), maxSearchNodes, numSamples, discountFactor);
      context.resultCallback(T("uniform"), minDepthScore);
      
      double optimisticScore = evaluateSearchHeuristic(context, system, optimisticPlanningSearchHeuristic(discountFactor), maxSearchNodes, numSamples, discountFactor);
      context.resultCallback(T("optimistic"), optimisticScore);
      
      context.leaveScope(true);
    }
    return true;
  }

  double evaluateSearchHeuristic(ExecutionContext& context, SequentialDecisionProblemPtr problem, SearchHeuristicPtr heuristic, size_t maxSearchNodes, size_t numSamples, double discount)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating heuristic ") + heuristic->toShortString(), numSamples);
    std::vector<double> results(numSamples);
    for (size_t i = 0; i < numSamples; ++i)
    {
      Variable state = problem->sampleInitialState(random);
      workUnit->setWorkUnit(i, new EvaluateSearchHeuristicWorkUnit(problem, state, heuristic, maxSearchNodes, discount, results[i]));
    }
    workUnit->setProgressionUnit(T("Samples"));
    workUnit->setPushChildrenIntoStackFlag(false);
    context.run(workUnit);

    ScalarVariableStatistics stats;
    stats.push(results);

    //context.informationCallback(stats.toString());
    return stats.getMean();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
