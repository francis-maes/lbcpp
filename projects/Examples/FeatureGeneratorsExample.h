/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorsExample.h     | Illustrates simple Feature      |
| Author  : Francis Maes                   |  Generators                     |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
# define LBCPP_EXAMPLES_FEATURE_GENERATORS_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class FeatureGeneratorsExample : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    Variable myBoolean(true);
    Variable myProb(0.5, probabilityType);
    Variable myInteger1(51, positiveIntegerType);
    Variable myInteger2(2, positiveIntegerType);
    Variable myIntegerPair(new Pair(myInteger1, myInteger2));

    // Atomic
    testPerception(context, T("boolean"), booleanFeatures(), myBoolean);
    testPerception(context, T("probability"), defaultProbabilityFeatures(), myProb);
    testPerception(context, T("integer1"), defaultPositiveIntegerFeatures(), myInteger1);

    // Composed
    CompositePerceptionPtr composite = compositePerception(positiveIntegerType, T("combo"));
    composite->addPerception(T("base20"), defaultPositiveIntegerFeatures(20));
    composite->addPerception(T("base2"), defaultPositiveIntegerFeatures(2));
    testPerception(context, T("integer2"), composite, myInteger2);

    // Product and conjunction
    FunctionPtr makePairFunction = identityFunction(pairClass(anyType, anyType));
    testPerception(context, T("pair (I1,I2)"), productPerception(makePairFunction, defaultPositiveIntegerFeatures(), composite, false), myIntegerPair);
    testPerception(context, T("pair (I1,I2) features"), conjunctionFeatures(composite, composite, false), myIntegerPair);
    return true;
  }

private:
  void testPerception(ExecutionContext& context, const String& name, PerceptionPtr perception, const Variable& input)
  {
    std::cout << "=========== " << name << " ==============" << std::endl;
    Variable output = perception->computeFunction(context, input);
    output.printRecursively(std::cout, -1, false, false);
    std::cout << std::endl;
    context.resultCallback(name, output);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
