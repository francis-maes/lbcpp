/*-----------------------------------------.---------------------------------.
| Filename: ProductWeakLearner.h           | Product Weak Learner            |
| Author  : Francis Maes                   | From "Boosting products of base |
| Started : 02/11/2011 10:33               | classifiers", Kegl & Busa-Feket |
`------------------------------------------/  2009                           |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_PRODUCT_H_
# define LBCPP_LUAPE_WEAK_LEARNER_PRODUCT_H_

# include "LuapeBatchLearner.h"

namespace lbcpp
{

class LuapeProductNode : public LuapeNode
{
public:
  LuapeProductNode(const String& name, size_t numBaseNodes)
    : LuapeNode(booleanType, name), baseNodes(numBaseNodes) {}
  LuapeProductNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {
    bool res = true;
    for (size_t i = 0; i < baseNodes.size(); ++i)
    {
      bool prediction = !baseNodes[i] || baseNodes[i]->compute(context, state, callback).getBoolean();
      res = (res == prediction);
    }
    return res;
  }
  
  virtual size_t getDepth() const
  {
    size_t maxBaseDepth = 0;
    for (size_t i = 0; i < baseNodes.size(); ++i)
    {
      size_t d = baseNodes[i]->getDepth();
      if (d > maxBaseDepth)
        maxBaseDepth = d;
    }
    return maxBaseDepth + 1;
  }

  virtual String toShortString() const
  {
    String res(T("Product("));
    for (size_t i = 0; i < baseNodes.size(); ++i)
    {
      LuapeNodePtr baseNode = baseNodes[i];
      if (baseNode)
        res += baseNode->toShortString();
      else
        res += T("true");
      if (i < baseNodes.size() - 1)
        res += T(", ");
    }
    res += T(")");
    return res;
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    target.staticCast<LuapeProductNode>()->baseNodes = baseNodes;
    LuapeNode::clone(context, target);
  }

  LuapeNodePtr getBaseNode(size_t index) const
    {jassert(index < baseNodes.size()); return baseNodes[index];}

  void setBaseNode(size_t index, const LuapeNodePtr& node)
    {jassert(index < baseNodes.size()); baseNodes[index] = node;}
  
protected:
  std::vector<LuapeNodePtr> baseNodes;
  
  void computeCache(LuapeNodeCachePtr cache, bool isTrainingSamples)
  {
    size_t n = baseNodes[0]->getCache()->getNumSamples(isTrainingSamples);
    jassert(n);
    cache->resizeSamples(isTrainingSamples, n);
    for (size_t i = 0; i < n; ++i)
    {
      bool result = true;
      for (size_t j = 0; j < baseNodes.size(); ++j)
      {
        LuapeNodePtr baseNode = baseNodes[j];
        bool base = !baseNode || baseNode->getCache()->getSample(isTrainingSamples, i).getBoolean();
        result = (result == base);
      }
      cache->setSample(isTrainingSamples, i, result);
    }
  }  
};

typedef ReferenceCountedObjectPtr<LuapeProductNode> LuapeProductNodePtr;

#if 0 // FIXME

class ProductWeakLearner : public BoostingWeakLearner
{
public:
  ProductWeakLearner(BoostingWeakLearnerPtr baseLearner, size_t numBaseClassifiers)
    : baseLearner(baseLearner), numBaseClassifiers(numBaseClassifiers) {}
  ProductWeakLearner() {}
  
  Variable createInitialVote(const LuapeInferencePtr& function) const
  {
    LuapeClassifierPtr classifier = function.dynamicCast<LuapeClassifier>();
    if (classifier)
      return new DenseDoubleVector(classifier->getDoubleVectorClass(), (size_t)-1, 1.0);
    jassert(false);
    return Variable();
  }
  
  Variable computeVoteProduct(const std::vector<Variable>& votes) const
  {
    DenseDoubleVectorPtr vector = votes[0].dynamicCast<DenseDoubleVector>();
    jassert(vector);
    DenseDoubleVectorPtr res = vector->cloneAndCast<DenseDoubleVector>();
    size_t n = res->getNumValues();
    for (size_t i = 1; i < votes.size(); ++i)
    {
      DenseDoubleVectorPtr v = votes[i].getObjectAndCast<DenseDoubleVector>();
      for (size_t j = 0; j < n; ++j)
        res->setValue(j, res->getValue(j) * v->getValue(j));
    }
    return res;
  }
  
  BooleanVectorPtr createInitialPredictions(const LuapeInferencePtr& function) const
  {
    size_t n = function->getGraph()->getNumTrainingSamples();
    return new BooleanVector(n, true);
  }
  
  BooleanVectorPtr computePredictionsProduct(const std::vector<BooleanVectorPtr>& predictions) const
  {
    size_t n = predictions[0]->getNumElements();
    BooleanVectorPtr res(new BooleanVector(n));
    for (size_t i = 0; i < n; ++i)
    {
      bool p = true;
      for (size_t j = 0; j < predictions.size(); ++j)
        p = (p == predictions[j]->get(i));
      res->set(i, p);
    }
    return res;
  }
  
  ContainerPtr computeVirtualSupervision(const Variable& voteProduct, const BooleanVectorPtr& predictionsProduct,
                                             const Variable& baseVote, const BooleanVectorPtr& basePredictions,
                                             const ContainerPtr& sup) const
  {
    DenseDoubleVectorPtr voteProductDV = voteProduct.dynamicCast<DenseDoubleVector>();
    DenseDoubleVectorPtr baseVoteDV = baseVote.dynamicCast<DenseDoubleVector>();
    jassert(voteProductDV && baseVoteDV); // only implemented for the multi-class case yet    
    DenseDoubleVectorPtr voteSigns = new DenseDoubleVector(voteProductDV->getClass());
    for (size_t i = 0; i < voteSigns->getNumValues(); ++i)
      voteSigns->setValue(i, voteProductDV->getValue(i) / baseVoteDV->getValue(i));
    
    BooleanVectorPtr supervisions = sup.staticCast<BooleanVector>();
    
    size_t n = predictionsProduct->getNumElements();
    size_t m = voteProductDV->getNumElements();
    BooleanVectorPtr res = new BooleanVector(n * m);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      double sign = (predictionsProduct->get(i) ? 1.0 : -1.0) / (basePredictions->get(i) ? 1.0 : -1.0);
      for (size_t j = 0; j < m; ++j, ++index)
        res->set(index, (sign > 0.0) == supervisions->get(index));
    }
    return res;
  }
  
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    std::vector<BooleanVectorPtr> predictions(numBaseClassifiers, createInitialPredictions(function));
    BooleanVectorPtr predictionsProduct = computePredictionsProduct(predictions);
    
    std::vector<Variable> votes(numBaseClassifiers, createInitialVote(function));
    Variable voteProduct = computeVoteProduct(votes);
    
    LuapeProductNodePtr res = new LuapeProductNode(T("product"), numBaseClassifiers);
    
    double bestEdge = -DBL_MAX;
  
    for (size_t i = 0; true; ++i)
    {
      context.enterScope("Iteration " + String((int)i));
      context.resultCallback("iteration", i);
      
      size_t index = i % numBaseClassifiers;
      
      // compute base learner
      ContainerPtr virtualSupervisions = computeVirtualSupervision(voteProduct, predictionsProduct, votes[index], predictions[index], supervisions);
      std::vector<LuapeNodePtr> weak = baseLearner->learn(context, batchLearner, function, virtualSupervisions, weights);
      jassert(weak.size() == 1);
      context.resultCallback("baseNode", weak[0]);
      
      // update predictions
      //weak[0]->initialize(context, function->getGraph()->getCache());
      LuapeNodeCachePtr weakCache = weak[0]->getCache();
      predictions[index] = weakCache->getTrainingSamples().staticCast<BooleanVector>();
      predictionsProduct = computePredictionsProduct(predictions);
      
      // update votes
      LuapeNodePtr baseNodeBackup = res->getBaseNode(index);
      res->setBaseNode(index, weak[0]);
      BoostingWeakObjectivePtr edgeCalculator = batchLearner->createWeakObjective(examples);
      edgeCalculator->initialize(function, predictions[index], virtualSupervisions, weights);
      votes[index] = edgeCalculator->computeVote();
      voteProduct = computeVoteProduct(votes);

      // stopping criterion
      edgeCalculator = batchLearner->createWeakObjective(examples);
      edgeCalculator->initialize(function, predictionsProduct, supervisions, weights);
      double newEdge = edgeCalculator->computeObjective();
      context.resultCallback("edge", newEdge);
      if (newEdge <= bestEdge)
      {
        context.informationCallback("Stopping: new edge=" + String(newEdge) + " prev edge=" + String(bestEdge));
        res->setBaseNode(index, baseNodeBackup); // restore previous base node
        context.leaveScope();
        
        // FIXME: compute predictions for initial weak learners that are null
        break;
      }
      else
      {
        bestEdge = newEdge;
        context.leaveScope();
      }
    }
  
    return std::vector<LuapeNodePtr>(1, res);
  }

protected:
  friend class ProductWeakLearnerClass;

  BoostingWeakLearnerPtr baseLearner;
  size_t numBaseClassifiers;
};

#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_PRODUCT_H_
