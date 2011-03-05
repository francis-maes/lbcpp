/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorsExample.h     | Illustrates simple Feature      |
| Author  : Francis Maes                   |  Generators                     |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
# define LBCPP_EXAMPLES_FEATURE_GENERATORS_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class FeatureGeneratorsExample : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    Variable myBoolean(true);
    Variable myMissingBoolean = Variable::missingValue(booleanType);
    Variable myProb(0.5, probabilityType);
    Variable myInteger1(51, positiveIntegerType);
    Variable myInteger2(2, positiveIntegerType);
    Variable myIntegerPair(new Pair(myInteger1, myInteger2));

    // Atomic
    testFeatureGenerator(context, T("boolean"), booleanFeatureGenerator(), myBoolean);
    testFeatureGenerator(context, T("missing boolean"), booleanFeatureGenerator(), myMissingBoolean);
    
    testFeatureGenerator(context, T("probability"), defaultProbabilityFeatureGenerator(), myProb);
    testFeatureGenerator(context, T("integer1"), defaultPositiveIntegerFeatureGenerator(), myInteger1);
    return Variable();
  }

private:
  void testFeatureGenerator(ExecutionContext& context, const String& name, FeatureGeneratorPtr featureGenerator, const Variable& input)
  {
    featureGenerator->initialize(context, input.getType());

    std::cout << "=========== " << name << " ==============" << std::endl;
    Variable output = featureGenerator->computeFunction(context, &input);
    output.printRecursively(std::cout, -1, false, false);
    std::cout << std::endl;
    context.resultCallback(name, output);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
