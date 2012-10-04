/*-----------------------------------------.---------------------------------.
| Filename: MCGPSandBox.h                  | Monte Carlo Genetic Programming |
| Author  : Francis Maes                   | SandBox                         |
| Started : 03/10/2012 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SANDBOX_H_
# define LBCPP_MCGP_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Luape/LuapeCache.h>
# include <lbcpp/Luape/ExpressionBuilder.h>
# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionSet.h>
# include <lbcpp-ml/ExpressionDomain.h>

# include <climits>

#if 0

namespace ocaml
{

using lbcpp::Object;
using lbcpp::ReferenceCountedObjectPtr;

class TypeParam;
typedef ReferenceCountedObjectPtr<TypeParam> TypeParamPtr;
class TypeParams;
typedef ReferenceCountedObjectPtr<TypeParams> TypeParamsPtr;
class TypeDefinition;
typedef ReferenceCountedObjectPtr<TypeDefinition> TypeDefinitionPtr;
class TypeDefinitions;
typedef ReferenceCountedObjectPtr<TypeDefinitions> TypeDefinitionsPtr;

class TypeExpression;
typedef ReferenceCountedObjectPtr<TypeExpression> TypeExpressionPtr;

class TypeParam : public Object
{
private:
  String identifer;
};

class TypeParams : public Object
{
public:

private:
  std::vector<TypeParamPtr> param;
};

// typedef ::=	[type-params] typeconstr-name [type-information]  
class TypeDefinition : public Object
{
private:
  std::vector<TypeParams> params;
  String typeConstructorName;
  // FIXME: type-information
};

// type-definition ::=	type typedef  { and typedef }  
class TypeDefinitions : public Object
{
public:

private:
  std::vector<TypeDefinitionPtr> typeDefs;
};

//////////////////////////

class TypeExpression : public Object
{
public:
};

// typexpr ::= 'ident | _
class IdentTypeExpression : public TypeExpression
{
public:

private:
  String ident;
};

// typeexpr ::= typeexpr -> typeexpr
class FunctionTypeExpression : public TypeExpression
{
private:
  TypeExpressionPtr left;
  TypeExpressionPtr right;
};

// typeexpr ::= typexpr  { * typexpr }+  
class TupleTypeExpression : public TypeExpression
{
private:
  std::vector<TypeExpressionPtr> types;
};

// typeexpr ::= typexpr typeconstr  |  ( typexpr  { , typexpr } ) typeconstr  
class ConstructedTypeExpression : public TypeExpression
{
private:
  std::vector<TypeExpressionPtr> arguments;
  String constructor;
};

}; /* namespace ocaml */

#endif // 0
#if 0

Example1:



let x = [Flag10; Flag3; Flag6]

Example2:

type simupol = RandomSimulationPolicy
type mcalgo = Simulate of simupol | Repeat of mcalgo * int | Step of mcalgo ...

#endif // 0


namespace lbcpp
{

/*
** Nil
*/
class NilObject : public Object
{
public:
};

typedef ReferenceCountedObjectPtr<NilObject> NilObjectPtr;

NilObjectPtr nilObject;

class NilDomain : public Domain
{
public:
  virtual SamplerPtr createDefaultSampler() const;
};

typedef ReferenceCountedObjectPtr<NilDomain> NilDomainPtr;

NilDomainPtr nilDomain;

class NilSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {}

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return nilObject;}

  virtual bool isDeterministic() const
    {return true;}
};

inline SamplerPtr NilDomain::createDefaultSampler() const
  {return new NilSampler();}

/*
** Integer
*/
class IntegerDomain : public Domain
{
public:
  IntegerDomain(int lowestValue = INT_MIN, int highestValue = INT_MAX)
    : lowestValue(lowestValue), highestValue(highestValue) {}

protected:
  friend class IntegerDomainClass;

  int lowestValue;
  int highestValue;
};

/*
** Double
*/
class DoubleObject : public Object
{
public:
  DoubleObject() : value(0.0) {}

private:
  double value;
};

class DoubleDomain : public Domain
{
public:
  DoubleDomain(double lowestValue = -DBL_MAX, double highestValue = DBL_MAX)
    : lowestValue(lowestValue), highestValue(highestValue) {}

  bool isFinite() const
    {return lowestValue != -DBL_MAX && highestValue != DBL_MAX;}

  virtual SamplerPtr createDefaultSampler() const;

private:
  friend class DoubleDomainClass;

  double lowestValue;
  double highestValue;
};

typedef ReferenceCountedObjectPtr<DoubleDomain> DoubleDomainPtr;

class UniformDoubleSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<DoubleDomain>();
    jassert(this->domain->isFinite());
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return nilObject;}

  virtual bool isDeterministic() const
    {return true;}

protected:
  DoubleDomainPtr domain;
};

inline SamplerPtr DoubleDomain::createDefaultSampler() const
{
  return isFinite() ? new UniformDoubleSampler() : SamplerPtr();
}

/*
** Tuple
*/
class TupleObject : public Object
{
public:
  TupleObject(const std::vector<ObjectPtr>& elements)
    : elements(elements) {}

private:
  std::vector<ObjectPtr> elements;
};

class TupleDomain : public Domain
{
public:
  TupleDomain(const DomainPtr& domain1, const DomainPtr& domain2, const DomainPtr& domain3)
    : elements(3)
  {
    elements[0] = domain1;
    elements[1] = domain2;
    elements[2] = domain3;
  }

  TupleDomain(const DomainPtr& domain1, const DomainPtr& domain2)
    : elements(2)
  {
    elements[0] = domain1;
    elements[1] = domain2;
  }

  void addElement(const DomainPtr& domain)
    {elements.push_back(domain);}

  size_t getNumElements() const
    {return elements.size();}

  DomainPtr getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

protected:
  std::vector<DomainPtr> elements;
};

typedef ReferenceCountedObjectPtr<TupleDomain> TupleDomainPtr;

class IndependentTupleSampler : public Sampler
{
public:
  IndependentTupleSampler(const std::vector<SamplerPtr>& samplers)
    : samplers(samplers) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<TupleDomain>();
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->initialize(context, this->domain->getElement(i));
  }
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    std::vector<ObjectPtr> elements(domain->getNumElements());
    for (size_t i = 0; i < elements.size(); ++i)
      elements[i] = samplers[i]->sample(context);
    return new TupleObject(elements);
  }

protected:
  std::vector<SamplerPtr> samplers;
  TupleDomainPtr domain;
};

/*
** Variant
*/
class VariantDomain : public Domain
{
public:
  void addConstructor(const String& identifier, const DomainPtr& domain = nilDomain)
    {constructors.push_back(std::make_pair(identifier, domain));}

  size_t getNumConstructors() const
    {return constructors.size();}

  const String& getConstructorName(size_t index) const
    {jassert(index < constructors.size()); return constructors[index].first;}

protected:
  std::vector< std::pair<String, DomainPtr> > constructors;
};

typedef ReferenceCountedObjectPtr<VariantDomain> VariantDomainPtr;

class VariantObject : public Object
{
public:
  VariantObject(const String& constructor, const ObjectPtr& argument)
    : constructor(constructor), argument(argument) {}

private:
  String constructor;
  ObjectPtr argument;
};

class UniformVariantSampler : public Sampler
{
public:
  UniformVariantSampler(const std::vector<SamplerPtr>& subSamplers)
    : subSamplers(subSamplers) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<VariantDomain>();
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    size_t constructorIndex = context.getRandomGenerator()->sampleSize(domain->getNumConstructors());
    return new VariantObject(domain->getConstructorName(constructorIndex), ObjectPtr());
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
    {jassertfalse;}

protected:
  std::vector<SamplerPtr> subSamplers;
  VariantDomainPtr domain;
};

/*
** -----------------------------------------------
*/
class Toto
{
public:
  DomainPtr makeColoDomain() const
  {
    // type flag = Flag1 | Flag2 | ... | Flag50
    // type flag_list = Nil | Cons of flag * flag_list

    VariantDomainPtr flagDomain = new VariantDomain();
    for (size_t i = 0; i < 50; ++i)
      flagDomain->addConstructor(String((int)i));
    VariantDomainPtr res = new VariantDomain();
    res->addConstructor("Nil");
    res->addConstructor("Cons", new TupleDomain(flagDomain, res));
    return res;
  }

  DomainPtr makeSymbolicRegressionDomain() const
  {
    // type expr = X | Constant of double | Log of expr | Exp of expr ... | Div of expr * expr

    VariantDomainPtr res = new VariantDomain();
    res->addConstructor("X");
    res->addConstructor("Constant", new DoubleDomain());
    res->addConstructor("Log", res);
    res->addConstructor("Exp", res);
    res->addConstructor("Sin", res);
    res->addConstructor("Cos", res);
    res->addConstructor("Add", new TupleDomain(res, res));
    res->addConstructor("Sub", new TupleDomain(res, res));
    res->addConstructor("Mul", new TupleDomain(res, res));
    res->addConstructor("Div", new TupleDomain(res, res));
    return res;
  }
};

// x + log(1.0)
// X 1.0 Constant Log Add EOF

class ExpressionProblem : public Problem
{
public:
  ExpressionProblem()
  {
    domain = new ExpressionDomain();

    std::vector< std::pair<double, double> > limits(2);
    limits[0] = std::make_pair(-DBL_MAX, DBL_MAX);
    limits[1] = std::make_pair(0, DBL_MAX); // expression size
    this->limits = new FitnessLimits(limits);
  }

  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ExpressionPtr();}

protected:
  ExpressionDomainPtr domain;
  FitnessLimitsPtr limits;
};

class F8SymbolicRegressionProblem : public ExpressionProblem
{
public:
  F8SymbolicRegressionProblem(size_t functionIndex = 0)
    : functionIndex(functionIndex)
  {
    // domain
		input = domain->addInput(doubleType, "x");

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
    limits->setLimits(0, 0.0, getWorstError());

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
      if (prediction == doubleMissingValue)
        prediction = 0.0;
      res.push(fabs(supervisions->getValue(it.getIndex()) - prediction));
    }

    // construct the Fitness
    std::vector<double> fitness(2);
    fitness[0] = res.getMean();
    fitness[1] = expression->getTreeSize();
    return new Fitness(fitness, limits);
  }

protected:
  friend class F8SymbolicRegressionProblemClass;

  size_t functionIndex;

  LuapeSamplesCachePtr cache;
  VariableExpressionPtr input;
  VariableExpressionPtr output;

  void getInputDomain(double& lowerLimit, double& upperLimit) const
  {
    lowerLimit = -1.0;
		upperLimit = 1.0;
		if (functionIndex == 7)
			lowerLimit = 0.0, upperLimit = 2.0;
		if (functionIndex == 8)
			lowerLimit = 0.0, upperLimit = 4.0;
  }

  double getWorstError() const
    {return 1.0;}

	double computeFunction(double x) const
	{
		double x2 = x * x;
		switch (functionIndex)
		{
		case 1: return x * x2 + x2 + x;
		case 2: return x2 * x2 + x * x2 + x2 + x;
		case 3: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 4: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 5: return sin(x2) * cos(x) - 1.0;
		case 6: return sin(x) + sin(x + x2);
		case 7: return log(x + 1) + log(x2 + 1);
		case 8: return sqrt(x);

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

#if 0
class RandomExpressionSampler : public Sampler
{
public:
  RandomExpressionSampler(size_t expressionSize)
    : expressionSize(expressionSize) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<ExpressionDomain>();
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
    {jassertfalse;}

protected:
  size_t expressionSize;

  ExpressionDomainPtr domain;

  bool sampleAction(ExecutionContext& context, ExpressionRPNTypeStatePtr typeState, ObjectPtr& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (!typeState)
      return false;

    std::vector<double> probabilities(3, 0.0);
    double Z = 0.0;
    if (typeState->hasPushActions())
      probabilities[0] = 1.0, ++Z;
    if (typeState->hasApplyActions())
      probabilities[1] = 1.0, ++Z;
    if (typeState->hasYieldAction())
      probabilities[2] = 1.0, ++Z;
    jassert(Z > 0.0);
    size_t actionKind = random->sampleWithProbabilities(probabilities, Z);

    switch (actionKind)
    {
    case 0: // push
      {
        static const size_t numTrials = 10;
        size_t numVariables = problem->getNumInputs() + problem->getNumActiveVariables();
        for (size_t trial = 0; trial < numTrials; ++trial)
        {
          size_t variableIndex = random->sampleSize(numVariables);
          ExpressionPtr variable = variableIndex < problem->getNumInputs()
            ? (ExpressionPtr)problem->getInput(variableIndex)
            : problem->getActiveVariable(variableIndex - problem->getNumInputs());
          if (typeState->hasPushAction(variable->getType()))
          {
            res = variable;
            return true;
          }
        }
        return false;
      }

    case 1: // apply
      {
        const std::vector<std::pair<FunctionPtr, ExpressionRPNTypeStatePtr> >& apply = typeState->getApplyActions();
        jassert(apply.size());
        if (apply.empty())
          return false;
        res = apply[random->sampleSize(apply.size())].first;
        return true;
      }

    case 2: // yield
      res = ObjectPtr();
      return true;
    };

    return false;
  }
};
#endif // 0

class MCGPSandBox : public WorkUnit
{
public:
  MCGPSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    ProblemPtr problem = new F8SymbolicRegressionProblem(0);
    SamplerPtr sampler; // FIXME
    OptimizerPtr optimizer = randomOptimizer(sampler, numEvaluations);
    ParetoFrontPtr pareto = optimizer->optimize(context, problem, (Optimizer::Verbosity)verbosity);
    context.resultCallback("pareto", pareto);
    return true;
  }

protected:
  friend class MCGPSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SANDBOX_H_
