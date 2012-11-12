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
		  domain->addInput(booleanClass, "b" + string((int)i));

		domain->addFunction(andBooleanFunction());
    domain->addFunction(orBooleanFunction());
    domain->addFunction(nandBooleanFunction());
    domain->addFunction(norBooleanFunction());

    domain->addTargetType(booleanClass);
    setDomain(domain);

    // data
    size_t numCases = (1 << numBits);
    TablePtr data = new Table(numCases);
    for (size_t i = 0; i < domain->getNumInputs(); ++i)
      data->addColumn(domain->getInput(i), booleanClass);
    VariableExpressionPtr supervision = domain->createSupervision(booleanClass, "y");
    data->addColumn(supervision, booleanClass);

		for (size_t i = 0; i < numCases; ++i)
		{
      size_t numActiveBits = 0;
      for (size_t j = 0; j < numBits; ++j)
      {
        bool isBitActive = (i & (1 << j)) != 0;
        data->setElement(i, j, new Boolean(isBitActive));
        if (isBitActive)
          ++numActiveBits;
      }
      data->setElement(i, numBits, new Boolean(numActiveBits % 2 == 1));
		}

    // objective
    addObjective(binaryAccuracyObjective(data, supervision));
  }

protected:
  friend class BooleanParityProblemClass;

  size_t numBits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_BOOLEAN_PARITY_PROBLEM_H_
