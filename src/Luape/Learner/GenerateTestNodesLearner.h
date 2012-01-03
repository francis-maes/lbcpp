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
  GenerateTestNodesLearner(LuapeNodeBuilderPtr nodeBuilder)
    : nodeBuilder(nodeBuilder) {}
  GenerateTestNodesLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node)
  {
    std::vector<LuapeNodePtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 100, weakNodes);
    if (!weakNodes.size())
    {
      context.errorCallback(T("No weak nodes"));
      return false;
    }

    const LuapeSequenceNodePtr& sequenceNode = node.staticCast<LuapeSequenceNode>();
    sequenceNode->reserveNodes(sequenceNode->getNumSubNodes() + weakNodes.size());

    IndexSetPtr subset;
    if (trainingCache->getNumSamples() > 100)
    {
      subset = new IndexSet();
      subset->randomlyExpandUsingSource(context, 100, trainingCache->getAllIndices());
    }
    else
      subset = trainingCache->getAllIndices();

    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      LuapeNodePtr condition = weakNodes[i];
      if (condition->getType() != booleanType)
      {
        LuapeSampleVectorPtr samples = trainingCache->getSamples(context, condition, subset);
        double threshold = samples->sampleElement(context.getRandomGenerator()).toDouble();
        condition = new LuapeFunctionNode(stumpLuapeFunction(threshold), condition); // bypass universe
      }

      LuapeNodePtr testNode = new LuapeTestNode(condition,
            new LuapeConstantNode(Variable::create(sequenceNode->getType())),
            new LuapeConstantNode(Variable::create(sequenceNode->getType())),
            new LuapeConstantNode(Variable::create(sequenceNode->getType())));
      sequenceNode->pushNode(context, testNode);
    }
    context.informationCallback(T("Num features: ") + String((int)sequenceNode->getNumSubNodes()));
    return sequenceNode;
  }

protected:
  friend class GenerateTestNodesLearnerClass;

  LuapeNodeBuilderPtr nodeBuilder;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GENERATE_TEST_NODES_H_
