/*-----------------------------------------.---------------------------------.
| Filename: MCProblem.h                    | Monte Carlo Problem             |
| Author  : Francis Maes                   |                                 |
| Started : 12/06/2012 15:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_PROBLEM_H_
# define LBCPP_LUAPE_MC_PROBLEM_H_

# include "MCObjective.h"

namespace lbcpp
{

class MCProblem : public Object
{
public:
	virtual std::pair<DecisionProblemStatePtr, MCObjectivePtr> getInstance(ExecutionContext& context, size_t instanceIndex) = 0;
};

typedef ReferenceCountedObjectPtr<MCProblem> MCProblemPtr;

class DecisionProblemMCProblem : public MCProblem
{
public:
	DecisionProblemMCProblem(DecisionProblemPtr decisionProblem = DecisionProblemPtr())
	  : decisionProblem(decisionProblem) {}

	struct Objective : public MCObjective
	{
    Objective(DecisionProblemPtr decisionProblem)
      : decisionProblem(decisionProblem) {}

    DecisionProblemPtr decisionProblem;

  	virtual void getObjectiveRange(double& worst, double& best) const
	    {worst = 0.0; best = decisionProblem->getMaxReward();}

		virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
		  {return finalState->getFinalStateReward();}
	};

	virtual std::pair<DecisionProblemStatePtr, MCObjectivePtr> getInstance(ExecutionContext& context, size_t instanceIndex)
  {
    ScopedLock _(instancesLock);
		if (instances.size() <= instanceIndex)
		{
			MCObjectivePtr objective = new Objective(decisionProblem);
			while (instances.size() <= instanceIndex)
        instances.push_back(std::make_pair(decisionProblem->sampleInitialState(context), objective));
		}
    return std::make_pair(instances[instanceIndex].first->cloneAndCast<DecisionProblemState>(), instances[instanceIndex].second);
	}

protected:
	friend class DecisionProblemMCProblemClass;

	DecisionProblemPtr decisionProblem;
  CriticalSection instancesLock;
	std::vector< std::pair<DecisionProblemStatePtr, MCObjectivePtr> > instances;
};

class F8SymbolicRegressionMCProblem : public MCProblem
{
public:
	F8SymbolicRegressionMCProblem(size_t maxDepth = 10)
  	: maxDepth(maxDepth) {}

	virtual std::pair<DecisionProblemStatePtr, MCObjectivePtr> getInstance(ExecutionContext& context, size_t instanceIndex)
	{
		if (instances.empty())
		{
			instances.resize(8);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				LuapeRegressorPtr problem = createInstance(context, i + 1);
				LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, maxDepth);
				DecisionProblemStatePtr initialState(new LuapeNodeBuilderState(problem, typeSearchSpace));
				instances[i] = std::make_pair(problem, initialState);
			}
		}
		LuapeRegressorPtr problem = instances[instanceIndex % 8].first;
		DecisionProblemStatePtr initialState = instances[instanceIndex % 8].second->cloneAndCast<DecisionProblemState>();
		MCObjectivePtr objective(new SymbolicRegressionMCObjective(problem, 1.0));
		return std::make_pair(initialState, objective);
  }

protected:
	friend class F8SymbolicRegressionMCProblemClass;

	size_t maxDepth;
	std::vector< std::pair<LuapeRegressorPtr, DecisionProblemStatePtr> > instances;

	LuapeRegressorPtr createInstance(ExecutionContext& context, size_t problemNumber) const
	{
		LuapeRegressorPtr regressor = new LuapeRegressor();

		regressor->addInput(doubleType, "x");

		regressor->addConstant(1.0);

		regressor->addFunction(logDoubleLuapeFunction());
		regressor->addFunction(expDoubleLuapeFunction());
		regressor->addFunction(sinDoubleLuapeFunction());
		regressor->addFunction(cosDoubleLuapeFunction());

		regressor->addFunction(addDoubleLuapeFunction());
		regressor->addFunction(subDoubleLuapeFunction());
		regressor->addFunction(mulDoubleLuapeFunction());
		regressor->addFunction(divDoubleLuapeFunction());

		RandomGeneratorPtr random = context.getRandomGenerator();
		std::vector<ObjectPtr> examples(20);

		double lowerLimit = -1.0;
		double upperLimit = 1.0;
		if (problemNumber == 7)
			lowerLimit = 0.0, upperLimit = 2.0;
		else if (problemNumber == 8)
			lowerLimit = 0.0, upperLimit = 4.0;

		for (size_t i = 0; i < examples.size(); ++i)
		{
			double x = lowerLimit + (upperLimit - lowerLimit) * i / (examples.size() - 1.0);// random->sampleDouble(lowerLimit, upperLimit);
			double y = computeFunction(problemNumber, x);
			examples[i] = new Pair(new DenseDoubleVector(singletonEnumeration, doubleType, 1, x), y);
		}
		regressor->setSamples(context, examples);
		regressor->getTrainingCache()->disableCaching();// setMaxSizeInMegaBytes(100);
		return regressor;
	}

	static double computeFunction(size_t problemNumber, double x)
	{
		double x2 = x * x;
		switch (problemNumber)
		{
		case 1: return x * x2 + x2 + x;
		case 2: return x2 * x2 + x * x2 + x2 + x;
		case 3: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 4: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 5: return sin(x2) * cos(x) - 1.0;
		case 6: return sin(x) + sin(x + x2);
		case 7: return log(x + 1) + log(x2 + 1);
		case 8: return sqrt(x);
		default: jassert(false); return 0.0;
		};
	}
};

class PrimeNumberMCProblem : public MCProblem
{
public:
	PrimeNumberMCProblem(size_t maxDepth = 10)
	  : maxDepth(maxDepth) {}

	struct Objective : public LuapeMCObjective
	{
		Objective(LuapeRegressorPtr regressor)
		: LuapeMCObjective(regressor) {}

	  virtual double evaluate(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& formula)
	  {
		  size_t primeCounter = 1;
		  bool isvalid = true;
		  while(isPrime(evaluateFormula(context, formula, primeCounter)))
		  {
			  primeCounter++;
			  if(primeCounter>99)
			  {isvalid = false; break;}
		  }

		  if(!isvalid)
			  primeCounter=1;

		  return primeCounter;
	  }

	  virtual bool isPrime(double value)
	  {
		  double root = pow(value, 0.5);
		  bool prime=true;

		  if(value<2.0)
			  return false;
		  if((value-(int)value)>1e-12)
			  return false;
		  if(value==DBL_MAX)
			  return false;

		  for (size_t i=2; i<= root; i++)
			  if ((int)value % i == 0)
				  prime = false;
		  return prime;
	  }

    virtual void getObjectiveRange(double& worst, double& best) const
  	  {worst = 0.0; best = 100.0;}

	  // evaluateFormula() is a utility function to evaluate the formula given an integer input
	  double evaluateFormula(ExecutionContext& context, const LuapeNodePtr& formula, int input) const
	  {
		  Variable in((double)input);
		  double res = formula->compute(context, &in).getDouble();
		  return res == doubleMissingValue ? 0.0 : res;
	  }
  };

	virtual std::pair<DecisionProblemStatePtr, MCObjectivePtr> getInstance(ExecutionContext& context, size_t instanceIndex)
  {
		LuapeRegressorPtr regressor = new LuapeRegressor();
		regressor->addInput(doubleType, "x");

		// TODO: setup constants and functions here
		regressor->addConstant(1.0);
		regressor->addConstant(2.0);
		regressor->addConstant(3.0);
		regressor->addFunction(addDoubleLuapeFunction());
		regressor->addFunction(subDoubleLuapeFunction());
		regressor->addFunction(mulDoubleLuapeFunction());
		regressor->addFunction(divDoubleLuapeFunction());

		LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = regressor->getSearchSpace(context, maxDepth);
		DecisionProblemStatePtr initialState(new LuapeNodeBuilderState(regressor, typeSearchSpace));
		return std::make_pair(initialState, new Objective(regressor));
  }

protected:
	friend class PrimeNumberMCProblemClass;

	size_t maxDepth;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_PROBLEM_H_
