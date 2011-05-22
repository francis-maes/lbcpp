/*-----------------------------------------.---------------------------------.
| Filename: GPSandBox.h                    | Genetic Programming SandBox     |
| Author  : Francis Maes                   |                                 |
| Started : 22/05/2011 19:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
# define LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

enum GPOperator
{
  gpAddition,
  gpSubtraction,
  gpMultiplication,
  gpDivision,
};

extern EnumerationPtr gpOperatorEnumeration;

enum GPPre
{
  gpSin,
  gpCos,
  gpExp,
  gpLog
};

extern EnumerationPtr gpPreEnumeration;

class GPExpr : public Object
{
public:
  virtual double compute(double x) const = 0;
};

typedef ReferenceCountedObjectPtr<GPExpr> GPExprPtr;
extern ClassPtr gpExprClass;

class BinaryGPExpr : public GPExpr
{
public:
  virtual double compute(double x) const
  {
    double l = left->compute(x);
    double r = right->compute(x);
    switch (op)
    {
    case gpAddition: return l + r;
    case gpSubtraction: return l - r;
    case gpMultiplication: return l * r;
    case gpDivision: return r ? l / r : 0.0;
    default: jassert(false); return 0.0;
    }
  }
protected:
  friend class BinaryGPExprClass;

  GPExprPtr left;
  GPOperator op;
  GPExprPtr right;
};

class UnaryGPExpr : public GPExpr
{
public:
  virtual double compute(double x) const
  {
    double e = expr->compute(x);
    switch (pre)
    {
    case gpSin: return sin(e);
    case gpCos: return cos(e);
    case gpExp: return exp(e);
    case gpLog: return e <= 0 ? 0.0 : log(e);
    default: jassert(false); return 0.0;
    };
  }

protected:
  friend class UnaryGPExprClass;

  GPPre pre;
  GPExprPtr expr;
};

class VariableGPExpr : public GPExpr
{
public:
  virtual double compute(double x) const
    {return x;}

protected:
  friend class VariableGPExprClass;

  size_t index;
};

// GPExpr -> Double
class GPObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPObjectiveFunction(const std::vector<std::pair<double, double> >& examples)
    : SimpleUnaryFunction(gpExprClass, doubleType), examples(examples) {}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExprPtr& expression = input.getObjectAndCast<GPExpr>();

    double res = 0.0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      double prediction = expression->compute(examples[i].first);
      res += fabs(prediction - examples[i].second);
    }
    return res / (double)examples.size();
  }
 
protected:
  std::vector<std::pair<double, double> > examples;
};

class GPSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    std::vector<std::pair<double, double> > examples;
    for (size_t i = 0; i < 100; ++i)
    {
      double x = random->sampleDouble(0.0, 1.0);
      double y = x + x * x * (1 + x);
      examples.push_back(std::make_pair(x, y));
    }

    FunctionPtr objective = new GPObjectiveFunction(examples);
    if (!objective->initialize(context, gpExprClass))
      return false;

    SamplerPtr sampler = createExpressionSampler();
    return optimize(context, sampler, objective);
  }

  SamplerPtr createExpressionSampler() const
  {
    
  }

  bool optimize(ExecutionContext& context, const SamplerPtr& sampler, const FunctionPtr& objective)
  {
    size_t numIterations = 100;
    size_t populationSize = 1000;
    size_t numBests = 100;

    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests);
    optimizer->compute(context, optimizerContext, optimizerState);

    return true;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
