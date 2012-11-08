/*-----------------------------------------.---------------------------------.
| Filename: SymbolicRegressionProblem.h    | Symbolic Regression Benchmarks  |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 16:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SYMBOLIC_REGRESSION_H_
# define LBCPP_MCGP_SYMBOLIC_REGRESSION_H_

# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

// TODO: factorize somewhere
class SymbolicRegressionObjective : public Objective
{
public:
  SymbolicRegressionObjective(LuapeSamplesCachePtr cache, VariableExpressionPtr output)
    : cache(cache), output(output) {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
     // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    LuapeSampleVectorPtr predictions = cache->getSamples(context, expression);
    DenseDoubleVectorPtr supervisions = cache->getNodeCache(output);
    
    // compute mean absolute error
    double squaredError = 0.0;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction = it.getRawDouble();
      if (prediction == doubleMissingValue || !isNumberValid(prediction))
        prediction = 0.0;
      double delta = supervisions->getValue(it.getIndex()) - prediction;
      squaredError += delta * delta;
    }
    squaredError /= (double)supervisions->getNumValues();

    // construct the Fitness
    return 1.0 / (1.0 + sqrt(squaredError));
  }

protected:
  LuapeSamplesCachePtr cache;
  VariableExpressionPtr output;
};

class KozaSymbolicRegressionProblem : public NewProblem
{
public:
  virtual void getInputDomain(double& lowerLimit, double& upperLimit)
    {lowerLimit = -1.0; upperLimit = 1.0;}

  virtual double computeFunction(double x) const = 0;

  virtual void initialize(ExecutionContext& context)
  {
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInput(doubleType, "x");

		domain->addFunction(addDoubleFunction());
		domain->addFunction(subDoubleFunction());
		domain->addFunction(mulDoubleFunction());
		domain->addFunction(protectedDivDoubleFunction());

		domain->addFunction(sinDoubleFunction());
		domain->addFunction(cosDoubleFunction());
		domain->addFunction(expDoubleFunction());
    domain->addFunction(protectedLogDoubleFunction());
    setDomain(domain);

    VariableExpressionPtr output = domain->createSupervision(doubleType, "y");
    
    // data
    const size_t numSamples = 20;
    LuapeSamplesCachePtr cache = domain->createCache(numSamples);
    DenseDoubleVectorPtr supervisionValues = new DenseDoubleVector(numSamples, 0.0);
    double lowerLimit, upperLimit;
    getInputDomain(lowerLimit, upperLimit);
		for (size_t i = 0; i < numSamples; ++i)
		{
			double x = context.getRandomGenerator()->sampleDouble(lowerLimit, upperLimit);
      double y = computeFunction(x);

      cache->setInputObject(domain->getInputs(), i, new DenseDoubleVector(1, x));
			supervisionValues->setValue(i, y);
		}
    cache->cacheNode(defaultExecutionContext(), output, supervisionValues, T("Supervision"), false);
    cache->recomputeCacheSize();
    cache->disableCaching();

    addObjective(new SymbolicRegressionObjective(cache, output));
  }
};

class QuarticSymbolicRegressionProblem : public KozaSymbolicRegressionProblem
{
public:
  QuarticSymbolicRegressionProblem()
    {initialize(defaultExecutionContext());}

  virtual double computeFunction(double x) const
    {return x * (x * (x * (x + 1.0) + 1.0) + 1.0);}
};

#if 0
class F8SymbolicRegressionProblem : public NewProblem
{
public:
  F8SymbolicRegressionProblem(size_t functionIndex)
    : functionIndex(functionIndex) {initialize(defaultExecutionContext());}
  F8SymbolicRegressionProblem() {}

  virtual void initialize(ExecutionContext& context)
  {
    jassert(functionIndex >= 0 && functionIndex < 8);
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInput(doubleType, "x");

		domain->addConstant(1.0);

		domain->addFunction(logDoubleFunction());
		domain->addFunction(expDoubleFunction());
		domain->addFunction(sinDoubleFunction());
		domain->addFunction(cosDoubleFunction());

		domain->addFunction(addDoubleFunction());
		domain->addFunction(subDoubleFunction());
		domain->addFunction(mulDoubleFunction());
		domain->addFunction(divDoubleFunction());
    setDomain(domain);

    VariableExpressionPtr output = domain->createSupervision(doubleType, "y");
    
    // fitness limits
    limits->setLimits(0, getWorstError(), 0.0); // absolute error: should be minimized

    // data
    const size_t numSamples = 20;
    LuapeSamplesCachePtr cache = domain->createCache(numSamples);
    DenseDoubleVectorPtr supervisionValues = new DenseDoubleVector(numSamples, 0.0);
    double lowerLimit, upperLimit;
    getInputDomain(lowerLimit, upperLimit);
		for (size_t i = 0; i < numSamples; ++i)
		{
			double x = lowerLimit + (upperLimit - lowerLimit) * i / (numSamples - 1.0);// random->sampleDouble(lowerLimit, upperLimit);
      double y = computeFunction(x);

      cache->setInputObject(domain->getInputs(), i, new DenseDoubleVector(1, x));
			supervisionValues->setValue(i, y);
		}
    cache->cacheNode(defaultExecutionContext(), output, supervisionValues, T("Supervision"), false);
    cache->recomputeCacheSize();
    cache->disableCaching();

    addObjective(new SymbolicRegressionObjective(cache, output));
  }

protected:
  friend class F8SymbolicRegressionProblemClass;

  size_t functionIndex;

  void getInputDomain(double& lowerLimit, double& upperLimit) const
  {
    lowerLimit = -1.0;
		upperLimit = 1.0;
		if (functionIndex == 6)
			lowerLimit = 0.0, upperLimit = 2.0;
		if (functionIndex == 7)
			lowerLimit = 0.0, upperLimit = 4.0;
  }

  double getWorstError() const
    {return 1.0;}

	double computeFunction(double x) const
	{
		double x2 = x * x;
		switch (functionIndex)
		{
		case 0: return x * x2 + x2 + x;
		case 1: return x2 * x2 + x * x2 + x2 + x;
		case 2: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 3: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 4: return sin(x2) * cos(x) - 1.0;
		case 5: return sin(x) + sin(x + x2);
		case 6: return log(x + 1) + log(x2 + 1);
		case 7: return sqrt(x);

/*
                case 1: return x*x2-x2-x;
                case 2: return x2*x2-x2*x-x2-x;
                case 3: return x2*x2 + sin(x);
                case 4: return cos(x2*x)+sin(x+1);
                case 5: return sqrt(x)+x2;
                case 6: return x2*x2*x2 +1;
                case 7: return sin(x2*x+x2);
                case 8: return log(x2*x+1)+x;
*/
		default: jassert(false); return 0.0;
		};
	}
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SYMBOLIC_REGRESSION_H_
