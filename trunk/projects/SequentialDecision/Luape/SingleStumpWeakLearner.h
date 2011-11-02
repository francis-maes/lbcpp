/*-----------------------------------------.---------------------------------.
| Filename: SingleStumpWeakLearner.h       | Single Stump Weak Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 27/10/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_
# define LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_

# include "LuapeBatchLearner.h"

namespace lbcpp
{

class SingleStumpWeakLearner : public LuapeWeakLearner
{
public:
  SingleStumpWeakLearner() {}

  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->clearScores();
    size_t numExamples = graph->getNumTrainingSamples();

    size_t bestVariable = (size_t)-1;
    double bestThreshold = 0.0;
    double bestEdge = -DBL_MAX;

    size_t n = graph->getNumNodes();
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (node->getType() == doubleType)
      {
        double edge;
        BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
        edgeCalculator->initialize(function, new BooleanVector(numExamples, true), supervisions, weights);
        double threshold = edgeCalculator->findBestThreshold(context, node, edge, false);
        if (edge > bestEdge)
        {
          bestEdge = edge;
          bestVariable = i;
          bestThreshold = threshold;
        }
      }
    }

    context.informationCallback("Best stump: " + graph->getNode(bestVariable)->toShortString() + " >= " + String(bestThreshold));
    context.informationCallback("Edge: " + String(bestEdge));

    LuapeNodePtr res = new LuapeFunctionNode(new StumpFunction(bestThreshold), std::vector<size_t>(1, bestVariable));
    return std::vector<LuapeNodePtr>(1, res);
  }
};


class CombinedStumpWeakLearner : public LuapeWeakLearner
{
public:
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->clearScores();

    LuapeNodePtr bestIntermediateNode;
    size_t bestVariable = (size_t)-1;
    double bestThreshold = 0.0;
    double bestEdge = -DBL_MAX;

    size_t n = graph->getNumNodes();
    std::vector<size_t> doubleNodes;
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (node->getType() == doubleType)
        doubleNodes.push_back(i);
    }
    
    context.enterScope("Single stump");
    String bestSingleStump;
    double bestSingleStumpEdge = -DBL_MAX;
    for (size_t i = 0; i < doubleNodes.size(); ++i)
    {
      LuapeNodePtr node = graph->getNode(doubleNodes[i]);
      double edge;
      double threshold = findBestThreshold(context, batchLearner, function, node, supervisions, weights, edge);
      if (edge > bestEdge)
      {
        bestEdge = edge;
        bestIntermediateNode = LuapeNodePtr();
        bestVariable = doubleNodes[i];
        bestThreshold = threshold;
      }
      if (edge > bestSingleStumpEdge)
      {
        bestSingleStumpEdge = edge;
        bestSingleStump = node->toShortString() + " >= " + String(threshold);
      }
    }
    context.informationCallback("Best: " + bestSingleStump + " [" + String(bestSingleStumpEdge) + "]");
    context.leaveScope(bestSingleStumpEdge);

    context.enterScope("products");
    String bestProductStump;
    double bestProductStumpEdge = -DBL_MAX;
    for (size_t i = 0; i < doubleNodes.size(); ++i)
      for (size_t j = 0; j < i; ++j)
      {
        std::vector<size_t> arguments(2);
        arguments[0] = i;
        arguments[1] = j;
        LuapeNodePtr node = new LuapeFunctionNode(new ProductFunction(), arguments);
        graph->pushNode(context, node);
        
        double edge;
        double threshold = findBestThreshold(context, batchLearner, function, node, supervisions, weights, edge);
        if (edge > bestEdge)
        {
          bestEdge = edge;
          bestIntermediateNode = node;
          bestVariable = (size_t)-1;
          bestThreshold = threshold;
        }
        if (edge > bestProductStumpEdge)
        {
          bestProductStumpEdge = edge;
          bestProductStump = node->toShortString() + " >= " + String(threshold);
        }
        
        graph->popNode();
      }
    context.informationCallback("Best: " + bestProductStump + " [" + String(bestProductStumpEdge) + "]");
    context.leaveScope(bestProductStumpEdge);
    
    std::vector<LuapeNodePtr> res;
    if (bestIntermediateNode)
    {
      res.push_back(bestIntermediateNode);
      res.push_back(new LuapeFunctionNode(new StumpFunction(bestThreshold), std::vector<size_t>(1, graph->getNumNodes()))); 
    }
    else
      res.push_back(new LuapeFunctionNode(new StumpFunction(bestThreshold), std::vector<size_t>(1, bestVariable))); 
    return res;
  }
  
protected:
  double findBestThreshold(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, const LuapeNodePtr& node, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double& edge) const
  {
    size_t numExamples = function->getGraph()->getNumTrainingSamples();
    BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
    edgeCalculator->initialize(function, new BooleanVector(numExamples, true), supervisions, weights);
    return edgeCalculator->findBestThreshold(context, node, edge, true);
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_
