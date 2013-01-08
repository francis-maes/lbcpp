/*-----------------------------------------.---------------------------------.
| Filename: ColoProblem.h                  | Compiler Optimization Level     |
| Author  : Francis Maes                   | Problem                         |
| Started : 08/01/2013 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COLO_PROBLEM_H_
# define LBCPP_COLO_PROBLEM_H_

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

  size_t getLength() const
    {return sequence.size();}

  size_t getFlag(size_t position) const
    {jassert(position < sequence.size()); return sequence[position];}

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

  virtual string toShortString() const
    {return objectiveNumber ? "speedup" : "computational cost";}

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

}; /* namespace lbcpp */

#endif // !LBCPP_COLO_PROBLEM_H_
