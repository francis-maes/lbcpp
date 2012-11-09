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

class BooleanParityProblem : public Problem
{
public:
  BooleanParityProblem(size_t numBits) : numBits(numBits)
    {initialize(defaultExecutionContext());}
  BooleanParityProblem() {}

  virtual void initialize(ExecutionContext& context)
  {
    ExpressionDomainPtr domain = new ExpressionDomain();
    for (size_t i = 0; i < numBits; ++i)
		  domain->addInput(booleanType, "b" + String((int)i));

		domain->addFunction(andBooleanFunction());
    domain->addFunction(orBooleanFunction());
    domain->addFunction(nandBooleanFunction());
    domain->addFunction(norBooleanFunction());

    domain->addTargetType(booleanType);
    setDomain(domain);

    VariableExpressionPtr output = domain->createSupervision(booleanType, "y");
    
    // data
    size_t numCases = (1 << numBits);
    LuapeSamplesCachePtr cache = domain->createCache(numCases);
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

    // objective
    addObjective(binaryAccuracyObjective(cache, output));
  }

protected:
  friend class BooleanParityProblemClass;

  size_t numBits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_BOOLEAN_PARITY_PROBLEM_H_
