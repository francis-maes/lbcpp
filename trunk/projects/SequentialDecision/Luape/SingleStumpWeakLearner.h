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

#if 0 // FIXME
class SingleStumpWeakLearner : public LuapeWeakLearner
{
public:
  SingleStumpWeakLearner() {}

  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
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

    LuapeNodePtr res = graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(bestThreshold), graph->getNode(bestVariable));
    return std::vector<LuapeNodePtr>(1, res);
  }
};


class CombinedStumpWeakLearner : public LuapeWeakLearner
{
public:
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    LuapeGraphPtr graph = function->getGraph();
    graph->clearScores();

    LuapeNodePtr bestIntermediateNode;
    size_t bestVariable = (size_t)-1;
    double bestThreshold = 0.0;
    double bestEdge = -DBL_MAX;

    static const size_t maxDepth = 3;

    size_t n = graph->getNumNodes();
    std::vector<size_t> doubleNodes;
    std::vector<size_t> booleanNodes;
    for (size_t i = 0; i < n; ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (node->getDepth() < maxDepth)
      {
        if (node->getType() == doubleType)
          doubleNodes.push_back(i);
        else if (node->getType() == booleanType)
          booleanNodes.push_back(i);
      }
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

    processBinaryFunction(context, getType("AddFunction"), true,
                          batchLearner, function, doubleNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);
    processBinaryFunction(context, getType("SubFunction"), true,
                          batchLearner, function, doubleNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);
    processBinaryFunction(context, getType("ProductFunction"), true,
                          batchLearner, function, doubleNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);
    processBinaryFunction(context, getType("DivideFunction"), true,
                          batchLearner, function, doubleNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);
    processBinaryFunction(context, getType("BooleanAndFunction"), true,
                          batchLearner, function, booleanNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);
    processBinaryFunction(context, getType("BooleanXorFunction"), true,
                          batchLearner, function, booleanNodes, supervisions, weights,
                          bestIntermediateNode, bestVariable, bestThreshold, bestEdge);

    String desc;
    std::vector<LuapeNodePtr> res;
    if (bestIntermediateNode)
    {
      res.push_back(bestIntermediateNode);
      if (bestIntermediateNode->getType() == doubleType)
      {
        res.push_back(graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(bestThreshold), bestIntermediateNode)); 
        desc = bestIntermediateNode->toShortString() + " >= " + String(bestThreshold);
      }
      else
        desc = bestIntermediateNode->toShortString();
    }
    else
    {
      res.push_back(graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(bestThreshold), graph->getNode(bestVariable))); 
      desc = String("node ") + String((int)bestVariable) + " >= " + String(bestThreshold);
    }
    context.informationCallback(desc + " [" + String(bestEdge) + "]");
    return res;
  }
  
protected:
  double findBestThreshold(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function, const LuapeNodePtr& node, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double& edge) const
  {
    size_t numExamples = function->getGraph()->getNumTrainingSamples();
    BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
    edgeCalculator->initialize(function, new BooleanVector(numExamples, true), supervisions, weights);
    return edgeCalculator->findBestThreshold(context, node, edge, false);
  }

  void processBinaryFunction(ExecutionContext& context, ClassPtr functionClass, bool isCommutative,
                                const BoostingLuapeLearnerPtr& batchLearner,
                                const LuapeInferencePtr& function, const std::vector<size_t>& nodeIndices,
                                const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, 
                                LuapeNodePtr& bestIntermediateNode, size_t& bestVariable, double& bestThreshold, double& bestEdge) const
  {
    context.enterScope(functionClass->getName());
    LuapeGraphPtr graph = function->getGraph();
    String localBest;
    double localBestEdge = -DBL_MAX;
    for (size_t i = 0; i < nodeIndices.size(); ++i)
      for (size_t j = 0; j < (isCommutative ? i : nodeIndices.size()); ++j)
      {
        std::vector<LuapeNodePtr> arguments(2);
        arguments[0] = graph->getNode(nodeIndices[i]);
        arguments[1] = graph->getNode(nodeIndices[j]);
        LuapeNodePtr node = graph->getUniverse()->makeFunctionNode(functionClass, std::vector<Variable>(), arguments);
        bool ok = graph->pushNode(context, node);
        jassert(ok);
        jassert(node->getCache());
        
        double edge;
        double threshold;
        if (node->getType() == doubleType)
        {
          threshold = findBestThreshold(context, batchLearner, function, node, supervisions, weights, edge);
        }
        else
        {
          jassert(node->getType() == booleanType);
          BoostingEdgeCalculatorPtr edgeCalculator = batchLearner->createEdgeCalculator();
          edgeCalculator->initialize(function, node->getCache()->getTrainingSamples().staticCast<BooleanVector>(), supervisions, weights);
          threshold = 0.0;
          edge = edgeCalculator->computeEdge();
        }

        if (edge > localBestEdge)
        {
          localBestEdge = edge;
          localBest = node->toShortString() + " >= " + String(threshold);
        }

        if (edge > bestEdge) // all times best edge
        {
          bestEdge = edge;
          bestIntermediateNode = node;
          bestVariable = (size_t)-1;
          bestThreshold = threshold;
        }
        
        graph->popNode();
      }
    context.informationCallback("Best: " + localBest + " [" + String(localBestEdge) + "]");
    context.leaveScope(localBestEdge);
  }
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_SINGLE_STUMP_H_
