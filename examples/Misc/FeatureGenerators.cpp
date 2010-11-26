/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerators.cpp          | Illustrates simple Feature      |
| Author  : Francis Maes                   |  Generators                     |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

void testPerception(ExecutionContext& context, const String& name, PerceptionPtr perception, const Variable& input)
{
  std::cout << "=========== " << name << " ==============" << std::endl;
  Variable output = perception->computeFunction(context, input);
  output.printRecursively(std::cout, -1, false, false);
  std::cout << std::endl;
}

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = defaultConsoleExecutionContext();

  Variable myBoolean(true);
  Variable myProb(0.5, probabilityType);
  Variable myInteger1(51, positiveIntegerType);
  Variable myInteger2(2, positiveIntegerType);
  Variable myIntegerPair(new Pair(myInteger1, myInteger2));

  // Atomic
  testPerception(*context, T("boolean"), booleanFeatures(), myBoolean);
  testPerception(*context, T("probability"), defaultProbabilityFeatures(), myProb);
  testPerception(*context, T("integer1"), defaultPositiveIntegerFeatures(), myInteger1);

  // Composed
  CompositePerceptionPtr composite = compositePerception(positiveIntegerType, T("combo"));
  composite->addPerception(T("base20"), defaultPositiveIntegerFeatures(20));
  composite->addPerception(T("base2"), defaultPositiveIntegerFeatures(2));
  testPerception(*context, T("integer2"), composite, myInteger2);

  // Product and conjunction
  FunctionPtr makePairFunction = identityFunction(pairClass(anyType, anyType));
  testPerception(*context, T("pair (I1,I2)"), productPerception(makePairFunction, defaultPositiveIntegerFeatures(), composite, false), myIntegerPair);
  testPerception(*context, T("pair (I1,I2) features"), conjunctionFeatures(composite, composite), myIntegerPair);
  return 0;
}
