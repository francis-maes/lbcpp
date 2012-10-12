/*-----------------------------------------.---------------------------------.
| Filename: BooleanParityProblem.h         | Boolean Parity Problem          |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_BOOLEAN_PARITY_PROBLEM_H_
# define LBCPP_MCGP_BOOLEAN_PARITY_PROBLEM_H_

# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{
  
class BooleanExpressionProblem : public ExpressionProblem
{
public:
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    jassert(cache);

    // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    LuapeSampleVectorPtr predictions = cache->getSamples(context, expression);
    BooleanVectorPtr supervisions = cache->getNodeCache(output).staticCast<BooleanVector>();
    
    // compute num successes
    size_t numSuccesses = 0;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      unsigned char supervision = supervisions->getData()[it.getIndex()];
      unsigned char prediction = it.getRawBoolean();
      if (supervision == prediction)
        ++numSuccesses;
    }

    //if (numSuccesses != 32)
    //  std::cout << expression->toShortString() << " => " << numSuccesses << std::endl;

    // construct the Fitness
    std::vector<double> fitness(1);
    fitness[0] = (double)numSuccesses;
    return new Fitness(fitness, limits);
  }

protected:
  LuapeSamplesCachePtr cache;
  VariableExpressionPtr output;
};

class BooleanParityProblem : public BooleanExpressionProblem
{
public:
  BooleanParityProblem(size_t numBits) : numBits(numBits)
    {initialize(defaultExecutionContext());}
  BooleanParityProblem() {}

  virtual void initialize(ExecutionContext& context)
  {
    domain = new ExpressionDomain();
    for (size_t i = 0; i < numBits; ++i)
		  domain->addInput(booleanType, "b" + String((int)i));

		domain->addFunction(andBooleanFunction());
    domain->addFunction(orBooleanFunction());
    domain->addFunction(nandBooleanFunction());
    domain->addFunction(norBooleanFunction());

    domain->addTargetType(booleanType);
    output = domain->createSupervision(booleanType, "y");
    
    // fitness limits
    size_t numCases = (1 << numBits);
    limits->setLimits(0, 0.0, (double)numCases); // number of correct cases, should be maximized

    // data
    cache = domain->createCache(numCases);
    BooleanVectorPtr supervisionValues = new BooleanVector(numCases);
		for (size_t i = 0; i < numCases; ++i)
		{
      BooleanVectorPtr input = new BooleanVector(numBits);
      size_t numActiveBits = 0;
      for (size_t j = 0; j < numBits; ++j)
      {
        bool isBitActive = (i & (1 << j)) != 0;
        input->set(j, isBitActive);
        if (isBitActive)
          ++numActiveBits;
      }
      cache->setInputObject(domain->getInputs(), i, input);
			supervisionValues->set(i, numActiveBits % 2 == 1);
		}
    cache->cacheNode(defaultExecutionContext(), output, supervisionValues, T("Supervision"), false);
    cache->recomputeCacheSize();
    cache->disableCaching();
  }

protected:
  friend class BooleanParityProblemClass;

  size_t numBits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_BOOLEAN_PARITY_PROBLEM_H_
