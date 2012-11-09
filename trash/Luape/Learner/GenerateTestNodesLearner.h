/*-----------------------------------------.---------------------------------.
| Filename: GenerateTestNodesLearner.h     | Generate test nodes             |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2011 23:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_GENERATE_TEST_NODES_H_
# define LBCPP_LUAPE_LEARNER_GENERATE_TEST_NODES_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class GenerateTestNodesLearner : public LuapeLearner
{
public:
  GenerateTestNodesLearner(ExpressionBuilderPtr nodeBuilder)
    : nodeBuilder(nodeBuilder) {}
  GenerateTestNodesLearner() {}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    std::vector<ExpressionPtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 100, weakNodes);
    if (!weakNodes.size())
    {
      context.errorCallback(T("No weak nodes"));
      return ExpressionPtr();
    }

    const SequenceExpressionPtr& sequenceNode = node.staticCast<SequenceExpression>();
    sequenceNode->reserveNodes(sequenceNode->getNumSubNodes() + weakNodes.size());

    IndexSetPtr subset = examples->size() > 100 ? examples->sampleSubset(context.getRandomGenerator(), 100) : examples;

    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      ExpressionPtr condition = weakNodes[i];
      if (condition->getType() != booleanType)
      {
        DataVectorPtr samples = problem->getTrainingCache()->getSamples(context, condition, subset);
        double threshold = samples->sampleElement(context.getRandomGenerator()).toDouble();
        condition = new FunctionExpression(stumpFunction(threshold), condition); // bypass universe
      }

      ExpressionPtr testNode = new TestExpression(condition,
            new ConstantExpression(Object::create(sequenceNode->getType())),
            new ConstantExpression(Object::create(sequenceNode->getType())),
            new ConstantExpression(Object::create(sequenceNode->getType())));
      sequenceNode->pushNode(context, testNode);
    }
    context.informationCallback(T("Num features: ") + String((int)sequenceNode->getNumSubNodes()));
    return sequenceNode;
  }

protected:
  friend class GenerateTestNodesLearnerClass;

  ExpressionBuilderPtr nodeBuilder;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GENERATE_TEST_NODES_H_
