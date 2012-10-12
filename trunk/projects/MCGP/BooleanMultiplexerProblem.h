/*-----------------------------------------.---------------------------------.
| Filename: BooleanMultiplexerProblem.h    | Boolean Multiplexer Problem     |
| Author  : Francis Maes                   |                                 |
| Started : 11/10/2012 11:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_
# define LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include "BooleanParityProblem.h"

namespace lbcpp
{

class BooleanMultiplexerProblem : public BooleanExpressionProblem
{
public:
  BooleanMultiplexerProblem(size_t numAddressBits) : numAddressBits(numAddressBits)
    {initialize();}
  BooleanMultiplexerProblem() {}

  virtual void initialize()
  {
    size_t numDataBits = (1 << numAddressBits);
    size_t numBits = numAddressBits + numDataBits;

    for (size_t i = 0; i < numAddressBits; ++i)
		  domain->addInput(booleanType, "a" + String((int)i));
    for (size_t i = 0; i < numDataBits; ++i)
      domain->addInput(booleanType, "d" + String((int)i));

		domain->addFunction(andBooleanFunction());
    domain->addFunction(orBooleanFunction());
    domain->addFunction(notBooleanFunction());
    domain->addFunction(ifThenElseBooleanFunction());

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
      size_t address = 0;
      for (size_t j = 0; j < numBits; ++j)
      {
        bool isBitActive = (i & (1 << j)) != 0;
        input->set(j, isBitActive);
        if (j < numAddressBits)
        {
          address <<= 1;
          if (isBitActive)
            address |= 1;
        }
      }
      cache->setInputObject(domain->getInputs(), i, input);
			supervisionValues->set(i, input->get(numAddressBits + address));
		}
    cache->cacheNode(defaultExecutionContext(), output, supervisionValues, T("Supervision"), false);
    cache->recomputeCacheSize();
    cache->disableCaching();
  }

protected:
  friend class BooleanMultiplexerProblemClass;

  size_t numAddressBits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_
