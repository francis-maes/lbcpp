/*-----------------------------------------.---------------------------------.
| Filename: ColoSandBox.h                  | Compiler Optimization Level     |
| Author  : Francis Maes                   | SandBox                         |
| Started : 14/09/2012 18:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_COLO_SANDBOX_H_
# define LBCPP_MOO_COLO_SANDBOX_H_

# include "MOOCore.h"
# include <lbcpp/Execution/WorkUnit.h>
# include "CrossEntropyOptimizer.h"
# include "DecoratorProblems.h"

namespace lbcpp
{

class ColoObject : public Object
{
public:
  void append(size_t flag)
    {sequence.push_back(flag);}

  const std::vector<size_t>& getSequence() const
    {return sequence;}

  virtual String toShortString() const
  {
    String res;
    for (size_t i = 0; i < sequence.size(); ++i)
    {
      if (i > 0)
        res += "-";
      res += String((int)sequence[i]);
    }
    return res;
  }

protected:
  friend class ColoObjectClass;

  std::vector<size_t> sequence;
};

typedef ReferenceCountedObjectPtr<ColoObject> ColoObjectPtr;

class ColoDomain : public MOODomain
{
public:
  size_t getNumFlags() const
    {return 33;}

  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const
    {return object;}
};

typedef ReferenceCountedObjectPtr<ColoDomain> ColoDomainPtr;

class ColoProblem : public MOOProblem
{
public:
  ColoProblem(const File& modelDirectory)
  {
    domain = new ColoDomain();
    std::vector< std::pair<double, double> > v(2);
    v[0].first = 10.0; v[1].first = 0.0; // computational cost (minimization)
    v[1].first = 0.0; v[1].second = 10.0; // speed-up (maximization)
    limits = new MOOFitnessLimits(v);

  }
  ColoProblem() {}

  virtual MOODomainPtr getObjectDomain() const
    {return domain;}

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    // FIXME
    std::vector<double> values(2);
    values[0] = context.getRandomGenerator()->sampleDouble(0.0, 10.0);
    values[1] = context.getRandomGenerator()->sampleDouble(0.0, 10.0);
    return new MOOFitness(values, limits);
  }

protected:
  friend class ColoProblemClass;

  File modelDirectory;

  ColoDomainPtr domain;
  MOOFitnessLimitsPtr limits;
};

class ColoSampler : public MOOSampler
{
public:
  virtual void initialize(ExecutionContext& context, const MOODomainPtr& domain)
  {
    probabilities.clear();
    size_t n = domain.staticCast<ColoDomain>()->getNumFlags() + 1;
    probabilities.resize(n, 1.0 / n);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    ColoObjectPtr res = new ColoObject();
    size_t maxLength = probabilities.size();
    for (size_t i = 0; i < maxLength; ++i)
    {
      size_t flag = random->sampleWithNormalizedProbabilities(probabilities);
      if (flag == probabilities.size() - 1)
        break;
      else
        res->append(flag);
    }
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
  {
    std::vector<size_t> counts(probabilities.size());
    size_t totalCount = 0;

    for (size_t i = 0; i < objects.size(); ++i)
    {
      const std::vector<size_t>& sequence = objects[i].staticCast<ColoObject>()->getSequence();
      for (size_t j = 0; j < sequence.size(); ++j)
        ++counts[sequence[j]];
      ++counts.back();
      totalCount += sequence.size() + 1;
    }

    for (size_t i = 0; i < probabilities.size(); ++i)
      probabilities[i] = counts[i] / (double)totalCount;
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& solution)
    {jassertfalse;}

protected:
  friend class ColoSamplerClass;

  std::vector<double> probabilities;
};

class ColoSandBox : public WorkUnit
{
public:
  ColoSandBox() : numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    MOOProblemPtr problem = new ColoProblem(modelDirectory);
    MOOOptimizerPtr optimizer = new CrossEntropyOptimizer(new ColoSampler(), 100, 50, numEvaluations / 100, true);
    runOptimizer(context, problem, optimizer);
    return true;
  }

  void runOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    MOOParetoFrontPtr front = optimizer->optimize(context, decorator);
    context.resultCallback("front", front);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    std::vector<double> hyperVolumes = decorator->getHyperVolumes();
    std::vector<double> cpuTimes = decorator->getCpuTimes();

    for (size_t i = 0; i < hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getEvaluationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes[i]);
      context.resultCallback("cpuTime", cpuTimes[i]);
      context.leaveScope();
    }

    double hyperVolume = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
    context.informationCallback("HyperVolume: " + String(hyperVolume));

    context.leaveScope(hyperVolume);
  }

protected:
  friend class ColoSandBoxClass;

  File modelDirectory;
  size_t numEvaluations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_COLO_SANDBOX_H_
