
#ifndef LBCPP_DICHOTOMIC_OPTIMIZER_H_
# define LBCPP_DICHOTOMIC_OPTIMIZER_H_
# include <lbcpp/lbcpp.h>

using namespace lbcpp;

class DichotomicOptimizer : public Inference
{
public:
  DichotomicOptimizer(double minValue, double maxValue, StoppingCriterionPtr criterion) : minValue(minValue), maxValue(maxValue), stoppingCriterion(criterion) {}
  
  virtual TypePtr getInputType() const
    {return objectiveFunctionClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    jassert(stoppingCriterion);
    jassert(minValue <= maxValue);
    ObjectiveFunctionPtr objective = input.getObjectAndCast<ObjectiveFunction>();
    jassert(objective);

    double res = 0.0;
    while (!stoppingCriterion->shouldStop(res))
    {
      double mean = (maxValue + minValue) / 2;
      double step = (maxValue - minValue) / 4;
      double minBound = mean - step;
      double maxBound = mean + step;

      double firstResult = objective->compute(context, minBound);
      double secondResult = objective->compute(context, maxBound);

      if (firstResult > secondResult)
      {
        const_cast<DichotomicOptimizer* >(this)->maxValue = mean;
        res = firstResult;
      }
      else
      {
        const_cast<DichotomicOptimizer* >(this)->minValue = mean;
        res = secondResult;
      }
    }
    std::cout << "Result : " << res << std::endl;
    return res;
  }

protected:
  double minValue;
  double maxValue;
  StoppingCriterionPtr stoppingCriterion;
};

class PolychotomicOptimizer : public Inference
{
public:
  PolychotomicOptimizer(size_t numParts, double minValue, double maxValue, StoppingCriterionPtr criterion)
  : numParts(numParts), minValue(minValue), maxValue(maxValue), stoppingCriterion(criterion) {}

  virtual TypePtr getInputType() const
    {return objectiveFunctionClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}
  
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    jassert(stoppingCriterion);
    jassert(minValue <= maxValue);
    ObjectiveFunctionPtr objective = input.getObjectAndCast<ObjectiveFunction>();
    jassert(objective);
    
    double res = 0.0;
    while (!stoppingCriterion->shouldStop(res))
    {
      std::vector<double> results(numParts);
      double step = (maxValue - minValue) / (2 * numParts);

      for (size_t i = 0; i < numParts; ++i) // FIXME: launch jobs
      {
        results[i] = objective->compute(context, minValue + (i * 2 + 1) * step);
        //std::cout << "["<<(minValue + (i * 2 + 1) * step)<<"]: "<< results[i] << " ";
      }
      //std::cout << std::endl;
      
      double bestScore = -DBL_MAX;
      int bestPart = -1;
      for (size_t i = 0; i < numParts; ++i)
      {
        if (results[i] > bestScore)
        {
          bestScore = results[i];
          bestPart = i;
        }
      }
      jassert(bestPart != -1);
      const_cast<PolychotomicOptimizer* >(this)->minValue = minValue + (bestPart * 2) * step;
      const_cast<PolychotomicOptimizer* >(this)->maxValue = minValue + 2 * step;
      res = bestScore;
      //std::cout << "BestPart: " << bestPart << " BestScore: " << res << " minValue: " << minValue << " maxValue: " << maxValue << std::endl;
    }
    //std::cout << "Result : " << res << std::endl;
    return res;
  }

protected:
  size_t numParts;
  double minValue;
  double maxValue;
  StoppingCriterionPtr stoppingCriterion;
};

class DumbObjectiveFunction : public ObjectiveFunction
{
public:
  virtual TypePtr getInputType() const
    {return doubleType;}
  
  virtual double compute(ExecutionContext& context, const Variable& input) const
  {
    double x = input.getDouble();
    return -(x * x);
  }
};

class DumbOptimizerProgram : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    //InferencePtr optimizer = new DichotomicOptimizer(-5, 10, maxIterationsStoppingCriterion(11));
    InferencePtr optimizer = new PolychotomicOptimizer(2, -5, 10, maxIterationsStoppingCriterion(11));
    return runInference(context, optimizer, Variable(ObjectiveFunctionPtr(new DumbObjectiveFunction()), objectiveFunctionClass), Variable());
  }
};

#endif //!LBCPP_DICHOTOMIC_OPTIMIZER_H_
