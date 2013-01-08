/*-----------------------------------------.---------------------------------.
| Filename: ColoSandBox.h                  | Compiler Optimization Level     |
| Author  : Francis Maes                   | SandBox                         |
| Started : 14/09/2012 18:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COLO_SANDBOX_H_
# define LBCPP_COLO_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Problem.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

namespace lbcpp
{

class ColoObject : public Object
{
public:
  void append(size_t flag)
    {sequence.push_back(flag);}

  const std::vector<size_t>& getSequence() const
    {return sequence;}

  virtual string toShortString() const
  {
    string res;
    for (size_t i = 0; i < sequence.size(); ++i)
    {
      if (i > 0)
        res += "-";
      res += string((int)sequence[i]+1);
    }
    //std::cout << res << std::endl;
    return res;
  }

  virtual bool loadFromString(ExecutionContext& context, const string& str)
  {
    juce::StringArray tokens;
    tokens.addTokens(str, T("-"), NULL);
    sequence.resize(tokens.size());
    for (size_t i = 0; i < sequence.size(); ++i)
    {
      int val = tokens[i].getIntValue();
      jassert(val);
      sequence[i] = (size_t)(val - 1);
    }
    return true;
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

    ParetoFrontPtr referenceFront = makeReferenceParetoFront(context, problem);
    context.resultCallback("referenceFront", referenceFront);
    context.informationCallback("Reference HyperVolume: " + string(referenceFront->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness())));

    runOptimizer(context, problem, randomSolver(new ColoSampler(), numEvaluations));
    //runOptimizer(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, false));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler(), 100, 10, numEvaluations / 100, true));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, true));
    //runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, false));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 100, 10, numEvaluations / 100, true));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, true));
    //runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 300, numEvaluations / 1000, false));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 100, numEvaluations / 1000, true));
    runOptimizer(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 300, numEvaluations / 1000, true));
    
    /*runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, true));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, true));*/
    return new Boolean(true);
  }

  void runOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    DVectorPtr cpuTimes = new DVector();
    DVectorPtr hyperVolumes = new DVector();
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      hyperVolumeEvaluatorSolverCallback(evaluationPeriod, cpuTimes, hyperVolumes),
      maxEvaluationsSolverCallback(numEvaluations));

    optimizer->setVerbosity(verbosityProgressAndResult);
    optimizer->solve(context, problem, callback);

    for (size_t i = 0; i < hyperVolumes->getNumElements(); ++i)
    {
      size_t numEvaluations = (i + 1) * evaluationPeriod;
      context.enterScope(string((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("log(numEvaluations)", log10((double)numEvaluations));
      context.resultCallback("hyperVolume", hyperVolumes->get(i));
      context.resultCallback("cpuTime", cpuTimes->get(i));
      context.leaveScope();
    }

    double hyperVolume = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
    context.informationCallback("HyperVolume: " + string(hyperVolume));

    context.leaveScope(hyperVolume);
  }

protected:
  friend class ColoSandBoxClass;

  juce::File javaDirectory;
  juce::File modelDirectory;
  size_t numEvaluations;

  ParetoFrontPtr makeReferenceParetoFront(ExecutionContext& context, ProblemPtr problem)
  {
    static const char* sequences[] = {
      "32-12-31-32-26",
		  "32-12-31-32-16",
		  "9-29-31-31-32-19-12-16",
		  "32-12-2-9-29-31-31-32-19-12-16",
		  "32-12-2-9-29-12-16-9-23-32-12-31-32-26",
		  "32-12-2-9-29-12-27-31-32-31-32-32-27-19-12-27-3-2-16-29-12-19-16-19-23-26",
		  "13-12-31-32-16",
		  "13-19-12-33-27-12-16",
		  "13-19-12-32-12-2-9-29-31-31-32-19-12-16",
		  "20-29-12-19-16-19-26-13-16-30",
		  "31-2-9-29-12-27-13-31-31-32-19-12-16",
		  "13-31-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-2-9-29-31-31-32-19-12-16",
		  "12-2-12-31-29-8-2-29-13-16-9-11-23-30",
		  "13-31-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-23-12-2-9-29-12-27-24-16",
		  "32-12-2-9-13-16-9-11-23-12-2-9-29-12-27-24-16",
		  "32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "12-2-12-20-12-27-31-32-19-12-26-13-16-9-11-23-29-16-29-12-19-16-19-23-26",
		  "13-20-12-27-31-32-32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "20-29-12-27-31-32-32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "28-27-3-2-16-28-31-32-31-32-32-27-19-12-31-32-19-12-16",
		  "32-12-2-9-29-12-27-31-32-28-27-3-2-16-28-26",
		  "32-12-2-9-29-12-27-31-32-28-27-3-2-16-28-24-16",
		  "32-12-2-9-29-12-27-31-32-28-27-2-9-29-31-31-32-19-12-16",
		  "12-12-28-24-3-2-16-9-23-26",
		  "12-12-28-24-28-27-3-2-16-9-23-26",
		  "32-12-2-9-29-12-28-24-28-27-3-2-16-29-12-19-16-19-23-26",
		  "13-19-12-28-24-28-27-3-2-16-28-31-32-31-32-32-27-19-12-31-32-19-12-16",
		  "13-9-29-12-27-31-32-28-27-3-2-16-28-24-16",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-12-31-32-16",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-3-12-2-9-29-16",
		  "13-19-12-28-24-28-27-3-2-16-29-12-19-16-19-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "32-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "13-20-12-27-31-32-19-12-12-12-28-24-28-27-3-2-16-9-23-26",
		  "12-28-27-3-2-16-24-28-27-3-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-27-3-2-16-29-12-19-9-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-26-13-12-28-32-12-2-9-29-12-16-9-23-26",
		  "32-27-19-12-29-8-2-29-13-16-9-28-20-28-31-12-26-13-16-11-23-2-28-27-19-12-28-31-14-16",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-2-9-29-31-31-32-19-12-16",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-27-3-2-16-29-12-19-16-19-26-13-16-9-12-28-27-3-2-16-28-24-28-27-3-2-16-3-14-16-9-11-9-23-26",
		  "12-2-12-31-12-28-14-10-28-31-14-16",
		  "12-27-31-12-28-14-10-28-31-14-16",
		  "12-27-31-12-28-14-10-29-8-2-23-26",
		  "32-12-28-24-28-27-3-10-29-8-2-23-26",
		  "32-12-2-9-27-3-9-12-28-14-10-23-26",
		  "32-12-2-27-3-2-16-28-24-28-27-3-10-29-8-2-23-26",
		  "32-12-2-9-29-12-27-32-12-2-9-29-16-28-24-28-27-3-2-16-3-14-10-29-8-2-23-26",
		  "32-12-2-9-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-16-3-14-10-29-8-2-23-26",
		  "13-19-12-28-24-12-19-16-19-26-13-16-9-12-28-27-3-2-16-28-24-28-27-3-2-16-3-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-14-10-12-2-9-29-12-16-9-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-19-12-28-24-28-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-7-16-3-14-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-2-16-28-24-28-27-3-2-16-24-13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-7-16-3-14-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-26-13-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-14-10-29-8-2-23-26",
		  "32-12-2-9-27-27-3-27-28-27-3-2-16-9-19-4-12-26-13-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-10-29-8-2-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-3-10-29-8-2-23-8-2-29-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-10-29-8-2-23-26",
		  "13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-16-9-11-9-23-26"
    };

    ParetoFrontPtr res = new ParetoFront(problem->getFitnessLimits());
    for (size_t i = 0; i < sizeof (sequences) / sizeof (const char* ); ++i)
    {
      ColoObjectPtr sequence = new ColoObject();
      sequence->loadFromString(context, sequences[i]);
      FitnessPtr fitness = problem->evaluate(context, sequence);
      res->insertSolution(sequence, fitness);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_COLO_SANDBOX_H_
