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

class KozaSymbolicRegressionProblem : public Problem
{
public:
  virtual void getInputDomain(double& lowerLimit, double& upperLimit)
    {lowerLimit = -1.0; upperLimit = 1.0;}

  virtual double computeFunction(double x) const = 0;

  virtual void initialize(ExecutionContext& context)
  {
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInput(newDoubleClass, "x");

		domain->addFunction(addDoubleFunction());
		domain->addFunction(subDoubleFunction());
		domain->addFunction(mulDoubleFunction());
		domain->addFunction(protectedDivDoubleFunction());

		domain->addFunction(sinDoubleFunction());
		domain->addFunction(cosDoubleFunction());
		domain->addFunction(expDoubleFunction());
    domain->addFunction(protectedLogDoubleFunction());
    setDomain(domain);

    
    // data
    const size_t numSamples = 20;
    
    TablePtr data = new Table(numSamples);
    VariableExpressionPtr supervision = domain->createSupervision(newDoubleClass, "y");
    data->addColumn(domain->getInput(0), newDoubleClass);
    data->addColumn(supervision, newDoubleClass);

    double lowerLimit, upperLimit;
    getInputDomain(lowerLimit, upperLimit);
		for (size_t i = 0; i < numSamples; ++i)
		{
			double x = context.getRandomGenerator()->sampleDouble(lowerLimit, upperLimit);
      data->setElement(i, 0, new NewDouble(x));
      data->setElement(i, 1, new NewDouble(computeFunction(x)));
		}
    addObjective(normalizedRMSERegressionObjective(data, supervision));
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
class F8SymbolicRegressionProblem : public Problem
{
public:
  F8SymbolicRegressionProblem(size_t functionIndex)
    : functionIndex(functionIndex) {initialize(defaultExecutionContext());}
  F8SymbolicRegressionProblem() {}

  virtual void initialize(ExecutionContext& context)
  {
    jassert(functionIndex >= 0 && functionIndex < 8);
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInput(newDoubleClass, "x");

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

    // fitness limits
    limits->setLimits(0, getWorstError(), 0.0); // absolute error: should be minimized

    // data
    const size_t numSamples = 20;
    TablePtr data = new Table(numSamples);
    VariableExpressionPtr supervision = domain->createSupervision(newDoubleClass, "y");
    data->addColumn(domain->getInput(0));
    data->addColumn(supervision);

    double lowerLimit, upperLimit;
    getInputDomain(lowerLimit, upperLimit);
		for (size_t i = 0; i < numSamples; ++i)
		{
			double x = lowerLimit + (upperLimit - lowerLimit) * i / (numSamples - 1.0);// random->sampleDouble(lowerLimit, upperLimit);
      double y = computeFunction(x);

      data->setSample(i, 0, new Double(x));
      data->setSample(i, 1, new Double(computeFunction(x)));
		}
    addObjective(normalizedRMSERegressionObjective(data, supervision));
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
