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
  GenerateTestNodesLearner(BoostingWeakLearnerPtr conditionGenerator)
    : conditionGenerator(conditionGenerator) {}
  GenerateTestNodesLearner() {}

  virtual bool initialize(ExecutionContext& context)
    {return conditionGenerator->initialize(context, function);}

  virtual bool learn(ExecutionContext& context)
  {
    std::vector<LuapeNodePtr> weakNodes;
    if (!conditionGenerator->getCandidateWeakNodes(context, this, weakNodes))
      return false;
    if (!weakNodes.size())
    {
      context.errorCallback(T("No weak nodes"));
      return false;
    }

    const LuapeSequenceNodePtr& sequenceNode = getRootNode().staticCast<LuapeSequenceNode>();
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
    return true;
  }

protected:
  friend class GenerateTestNodesLearnerClass;

  BoostingWeakLearnerPtr conditionGenerator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GENERATE_TEST_NODES_H_
