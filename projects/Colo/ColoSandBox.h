/*-----------------------------------------.---------------------------------.
| Filename: ColoSandBox.h                  | Compiler Optimization Level     |
| Author  : Francis Maes                   | SandBox                         |
| Started : 14/09/2012 18:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COLO_SANDBOX_H_
# define LBCPP_COLO_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/Problem.h>
# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionContainer.h>

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
      res += String((int)sequence[i]+1);
    }
    return res;
  }

protected:
  friend class ColoObjectClass;

  std::vector<size_t> sequence;
};

typedef ReferenceCountedObjectPtr<ColoObject> ColoObjectPtr;

class ColoDomain : public Domain
{
public:
  size_t getNumFlags() const
    {return 33;}
};

typedef ReferenceCountedObjectPtr<ColoDomain> ColoDomainPtr;

extern void* createColoJavaWrapper(ExecutionContext& context, const juce::File& javaDirectory, const juce::File& modelDirectory);
extern std::vector<double> evaluateColoJavaWrapper(void* wrapper, const ColoObjectPtr& colo);
extern void freeColoJavaWrapper(void* wrapper);

class ColoObjective : public Objective
{
public:
  ColoObjective(void* wrapper, size_t objectiveNumber, double worstScore, double bestScore)
    : wrapper(wrapper), objectiveNumber(objectiveNumber), worstScore(worstScore), bestScore(bestScore) {}
  ColoObjective() : wrapper(NULL), objectiveNumber(0), worstScore(0.0), bestScore(0.0) {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = worstScore; best = bestScore;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    std::vector<double> scores = evaluateColoJavaWrapper(wrapper, object.staticCast<ColoObject>()); // FIXME: evaluate single objective at a time
    jassert(objectiveNumber < scores.size());
    return scores[objectiveNumber];
  }

protected:
  void* wrapper;
  size_t objectiveNumber;
  double worstScore;
  double bestScore;
};

class ColoProblem : public Problem
{
public:
  ColoProblem(ExecutionContext& context, const juce::File& javaDirectory, const juce::File& modelDirectory)
    : javaDirectory(javaDirectory), modelDirectory(modelDirectory)
    {initialize(context);}
  ColoProblem() {}
  
  virtual ~ColoProblem()
    {if (wrapper) freeColoJavaWrapper(wrapper);}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ColoDomain());
    wrapper = createColoJavaWrapper(context, javaDirectory, modelDirectory);
    if (wrapper)
    {
      addObjective(new ColoObjective(wrapper, 0, 2.0, 0.5)); // computational cost (minimization)
      addObjective(new ColoObjective(wrapper, 1, 0.5, 2.0)); // speed-up (maximization)
    }
  }

protected:
  friend class ColoProblemClass;

  juce::File javaDirectory;
  juce::File modelDirectory;
  void* wrapper;
};

typedef ReferenceCountedObjectPtr<ColoProblem> ColoProblemPtr;

class ColoSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
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

protected:
  friend class ColoSamplerClass;

  std::vector<double> probabilities;
};

class ColoSampler2 : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    probabilities.clear();
    size_t n = domain.staticCast<ColoDomain>()->getNumFlags() + 1;
    probabilities.resize(n, std::vector<double>(n, 1.0 / n));
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    ColoObjectPtr res = new ColoObject();
    size_t maxLength = probabilities.size();
    size_t n = probabilities.size();
    size_t currentState = n - 1;
    for (size_t i = 0; i < maxLength; ++i)
    {
      size_t flag = random->sampleWithNormalizedProbabilities(probabilities[currentState]);
      if (flag == n - 1)
        break;
      else
      {
        res->append(flag);
        currentState = flag;
      }
    }
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
  {
	  size_t n = probabilities.size();
    std::vector< std::vector<size_t> > counts(n, std::vector<size_t>(n, 0));
    std::vector<size_t> totalCounts(n, 0);

    for (size_t i = 0; i < objects.size(); ++i)
    {
      const std::vector<size_t>& sequence = objects[i].staticCast<ColoObject>()->getSequence();
      size_t currentState = n-1;
      for (size_t j = 0; j < sequence.size(); ++j)
      {
        ++counts[currentState][sequence[j]];
        ++totalCounts[currentState];
        currentState = sequence[j];
      }
      ++counts[currentState][n-1];
      ++totalCounts[currentState];
    }

    for (size_t i = 0; i < n; ++i)
    {
      if (totalCounts[i])
      {
        double invZ = 1.0 / totalCounts[i];
        for (size_t j = 0; j < n; ++j)
          probabilities[i][j] = counts[i][j] * invZ;
      }
      else
        probabilities[i] = std::vector<double>(n, 1.0 / n);
    }
  }

protected:
  friend class ColoSampler2Class;

  std::vector< std::vector<double> > probabilities;
};

class ColoSandBox : public WorkUnit
{
public:
  ColoSandBox() : numEvaluations(1000) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    ColoProblemPtr problem = new ColoProblem(context, javaDirectory, modelDirectory);
    if (!problem->getNumObjectives())
      return new Boolean(false);

    runOptimizer(context, problem, randomSolver(new ColoSampler(), numEvaluations));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, false));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, true));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, false));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, true));
    
    /*runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, true));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, true));*/
    return new Boolean(true);
  }

  void runOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    DenseDoubleVectorPtr cpuTimes = new DenseDoubleVector(0, 0.0);
    DenseDoubleVectorPtr hyperVolumes = new DenseDoubleVector(0, 0.0);
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront();
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      hyperVolumeEvaluatorSolverCallback(evaluationPeriod, cpuTimes, hyperVolumes),
      maxEvaluationsSolverCallback(numEvaluations));

    optimizer->setVerbosity(verbosityProgressAndResult);
    optimizer->solve(context, problem, callback);

    for (size_t i = 0; i < hyperVolumes->getNumValues(); ++i)
    {
      size_t numEvaluations = i * evaluationPeriod;
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes->getValue(i));
      context.resultCallback("cpuTime", cpuTimes->getValue(i));
      context.leaveScope();
    }

    double hyperVolume = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
    context.informationCallback("HyperVolume: " + String(hyperVolume));

    context.leaveScope(hyperVolume);
  }

protected:
  friend class ColoSandBoxClass;

  juce::File javaDirectory;
  juce::File modelDirectory;
  size_t numEvaluations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_COLO_SANDBOX_H_
