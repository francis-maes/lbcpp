/*-----------------------------------------.---------------------------------.
| Filename: SampleExpressionTrajectories.h | A workunit for testing the      |
| Author  : Francis Maes                   | expression states               |
| Started : 12/10/2012 10:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_
# define LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class SampleExpressionTrajectories : public WorkUnit
{
public:
  SampleExpressionTrajectories() : numExpressions(1000), maxExpressionSize(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
    {
      context.errorCallback("No problem defined");
      return false;
    }
    ExpressionDomainPtr domain = problem->getDomain();

    context.informationCallback(domain->toShortString());
    sampleTrajectories(context, "prefix", prefixExpressionState(domain, maxExpressionSize));
    sampleTrajectories(context, "postfix", postfixExpressionState(domain, maxExpressionSize));
    sampleTrajectories(context, "typed-postfix", typedPostfixExpressionState(domain, maxExpressionSize));
    return true;
  }

protected:
  friend class SampleExpressionTrajectoriesClass;

  ExpressionProblemPtr problem;
  size_t numExpressions;
  size_t maxExpressionSize;

  void sampleTrajectories(ExecutionContext& context, const String& name, ExpressionStatePtr initialState)
  {
    std::vector<size_t> countsPerSize;
    SamplerPtr sampler = randomSearchSampler();
    sampler->initialize(context, new SearchDomain(initialState));

    context.enterScope(name);
    context.resultCallback("initialState", initialState);
    context.enterScope("Sampling");
    for (size_t i = 0; i < numExpressions; ++i)
    {
      SearchTrajectoryPtr trajectory = sampler->sample(context).staticCast<SearchTrajectory>();
      ExpressionPtr expression = trajectory->getFinalState()->getConstructedObject().staticCast<Expression>();
      if (i < 20)
      {
        context.informationCallback(T("Trajectory: ") + trajectory->toShortString());
        context.informationCallback(T(" => ") + expression->toShortString());
      }
      size_t size = expression->getTreeSize();
      jassert(size > 0);
      if (countsPerSize.size() <= size)
        countsPerSize.resize(size + 1, 0);
      countsPerSize[size]++;
      context.progressCallback(new ProgressionState(i+1, numExpressions, "Expressions"));
    }
    context.leaveScope();

    context.enterScope("Size Histogram");
    for (size_t i = 1; i < countsPerSize.size(); ++i)
    {
      context.enterScope("Size " + String((int)i));
      context.resultCallback("size", i);
      context.resultCallback("probability", countsPerSize[i] / (double)numExpressions);
      context.leaveScope();
    }
    context.leaveScope();

    context.leaveScope();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_
