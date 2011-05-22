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

  virtual size_t size() const = 0;
};

typedef ReferenceCountedObjectPtr<GPExpr> GPExprPtr;
extern ClassPtr gpExprClass;
extern EnumerationPtr gpExprEnumerationEnumeration;

extern ClassPtr variableGPExprClass;
extern ClassPtr unaryGPExprClass;
extern ClassPtr binaryGPExprClass;

class BinaryGPExpr : public GPExpr
{
public:
  virtual size_t size() const
    {return left->size() + 1 + right->size();}

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

  virtual String toShortString() const
  {
    const char* names[] = {"+", "-", "*", "/"};
    return T("(") + (left ? left->toShortString() : String("<null>")) + T(")") +
            names[op] +
           T("(") + (right ? right->toShortString() : String("<null>")) + T(")");
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
  virtual size_t size() const
    {return 1 + expr->size();}

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

  virtual String toShortString() const
  {
    static const char* names[] = {"sin", "cos", "exp", "log"};
    return String(names[pre]) + T("(") + (expr ? expr->toShortString() : String("<null>")) + T(")");
  }

protected:
  friend class UnaryGPExprClass;

  GPPre pre;
  GPExprPtr expr;
};

class VariableGPExpr : public GPExpr
{
public:
  virtual size_t size() const
    {return 1;}

  virtual double compute(double x) const
    {return x;}

  virtual String toShortString() const
    {return T("x");}

protected:
  friend class VariableGPExprClass;

  size_t index;
};

class GPExprSampler : public CompositeSampler
{
public:
  GPExprSampler(DiscreteSamplerPtr classSampler, size_t depth)
    : classSampler(classSampler), depth(depth)
  {
  }

  GPExprSampler() {}

  virtual String toShortString() const
    {return classSampler->toShortString();}

  SamplerPtr createSubExprSampler() const
  {
    if (depth < 5)
      return new GPExprSampler(enumerationSampler(gpExprEnumerationEnumeration), depth + 1);
    else
      return constantSampler(new VariableGPExpr());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    if (samplers.empty())
    {
      // lazy creation of sub-samplers
      const_cast<GPExprSampler* >(this)->samplers.push_back(objectCompositeSampler(binaryGPExprClass,
        createSubExprSampler(), enumerationSampler(gpOperatorEnumeration),  createSubExprSampler()));
      const_cast<GPExprSampler* >(this)->samplers.push_back(objectCompositeSampler(unaryGPExprClass,
        enumerationSampler(gpPreEnumeration), createSubExprSampler()));
      const_cast<GPExprSampler* >(this)->samplers.push_back(objectCompositeSampler(variableGPExprClass,
        new ConstantSampler((size_t)0)));
      const_cast<GPExprSampler* >(this)->samplers.push_back(classSampler);
    }

    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();

    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers);
    subSamples.resize(numSamplers);
    subWeights.resize(numSamplers);

    VectorPtr classSamples = vector(gpExprEnumerationEnumeration, n);
    for (size_t i = 0; i < numSamplers - 1; ++i)
    {
      subSamples[i] = new ObjectVector(gpExprClass, 0);
      if (inputs)
        subInputs[i] = vector(inputs->getElementsType());
    }
    subInputs.back() = inputs;
    subSamples.back() = classSamples;

    for (size_t i = 0; i < n; ++i)
    {
      Variable element = samples->getElement(i);
      TypePtr type = element.getObject()->getClass();

      size_t target;
      if (type == binaryGPExprClass)
        target = 0;
      else if (type == unaryGPExprClass)
        target = 1;
      else if (type == variableGPExprClass)
        target = 2;
      else
        jassert(false);
      classSamples->setElement(i, Variable(target, gpExprEnumerationEnumeration));

      subSamples[target].staticCast<Vector>()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector>()->append(inputs->getElement(i));
    }
  }

protected:
  friend class GPExprSamplerClass;

  DiscreteSamplerPtr classSampler;
  size_t depth;
};


// GPExpr -> Double
class GPObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPObjectiveFunction(const std::vector<std::pair<double, double> >& examples, double lambda)
    : SimpleUnaryFunction(gpExprClass, doubleType), examples(examples), lambda(lambda) {}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExprPtr& expression = input.getObjectAndCast<GPExpr>();

    double res = 0.0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      double prediction = expression->compute(examples[i].first);
      res += fabs(prediction - examples[i].second);
    }
    return res / (double)examples.size() + lambda * expression->size();
  }
 
protected:
  double lambda;
  std::vector<std::pair<double, double> > examples;
};

class GPSandBox : public WorkUnit
{
public:
  GPSandBox() : lambda(0.01)
  {
    numIterations = 100;
    populationSize = 1000;
    numBests = 200;
  }

  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    std::vector<std::pair<double, double> > examples;
    for (size_t i = 0; i < 1000; ++i)
    {
      double x = random->sampleDouble(0.0, 100.0);
      double y = cos(x * x + x * cos(x));
      examples.push_back(std::make_pair(x, y));
    }

    FunctionPtr objective = new GPObjectiveFunction(examples, lambda);
    if (!objective->initialize(context, gpExprClass))
      return false;

    SamplerPtr sampler = createExpressionSampler();

    //for (size_t i = 0; i < 10; ++i)
    //  context.informationCallback(sampler->sample(context, random).toShortString());

    return optimize(context, sampler, objective);
  }

  SamplerPtr createExpressionSampler() const
    {return new GPExprSampler(enumerationSampler(gpExprEnumerationEnumeration), 1);}

  bool optimize(ExecutionContext& context, const SamplerPtr& sampler, const FunctionPtr& objective)
  {
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, 0.0, true);
    optimizer->compute(context, optimizerContext, optimizerState);

    return true;
  }

protected:
  friend class GPSandBoxClass;

  double lambda;
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
