/*-----------------------------------------.---------------------------------.
| Filename: F8SymbolicRegressionProblem.h  | Symbolic Regression Benchmarks  |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 16:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_F8_SYMBOLIC_REGRESSION_H_
# define LBCPP_MCGP_F8_SYMBOLIC_REGRESSION_H_

# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class F8SymbolicRegressionProblem : public ExpressionProblem
{
public:
  F8SymbolicRegressionProblem(size_t functionIndex)
    : functionIndex(functionIndex) {initialize();}
  F8SymbolicRegressionProblem() {}

  virtual void initialize()
  {
    jassert(functionIndex >= 0 && functionIndex < 8);
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

    output = domain->createSupervision(doubleType, "y");
    
    // fitness limits
    limits->setLimits(0, getWorstError(), 0.0); // absolute error: should be minimized

    // data
    const size_t numSamples = 20;
    cache = domain->createCache(numSamples);
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
  }

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    LuapeSampleVectorPtr predictions = cache->getSamples(context, expression);
    DenseDoubleVectorPtr supervisions = cache->getNodeCache(output);
    
    // compute mean absolute error
    ScalarVariableMean res;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction = it.getRawDouble();
      if (prediction == doubleMissingValue || !isNumberValid(prediction))
        prediction = 0.0;
      res.push(fabs(supervisions->getValue(it.getIndex()) - prediction));
    }

    // construct the Fitness
    std::vector<double> fitness(1);
    fitness[0] = res.getMean();
    //fitness[1] = expression->getTreeSize();

    //std::cout << ExpressionRPNSequence::fromNode(expression)->toShortString() << " => " << res.getMean() << std::endl;
    return new Fitness(fitness, limits);
  }

protected:
  friend class F8SymbolicRegressionProblemClass;

  size_t functionIndex;

  LuapeSamplesCachePtr cache;
  VariableExpressionPtr output;

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

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_F8_SYMBOLIC_REGRESSION_H_
