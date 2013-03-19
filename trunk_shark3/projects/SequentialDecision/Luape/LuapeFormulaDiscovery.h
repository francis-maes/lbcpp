/*-----------------------------------------.---------------------------------.
| Filename: LuapeFormulaDiscovery.h        | Luape Formula Discovery         |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2012 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FORMULA_DISCOVERY_H_
# define LBCPP_LUAPE_FORMULA_DISCOVERY_H_

# include <lbcpp/Optimizer/BanditPool.h>
# include <lbcpp/Execution/WorkUnit.h>
# include "MCAlgorithm.h"
# include "../GP/BanditFormulaSearchProblem.h"

namespace lbcpp
{

class ExpressionSearchProblem : public LuapeInference
{
public:
  virtual bool initializeProblem(ExecutionContext& context) = 0;

  virtual size_t getNumInstances() const {return 0;} // 0 stands for infinity
  virtual void getObjectiveRange(double& worst, double& best) const = 0;
  virtual double computeObjective(ExecutionContext& context, const ExpressionPtr& node, size_t instanceIndex) = 0;

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const ExpressionPtr& node) const = 0;
};

typedef ReferenceCountedObjectPtr<ExpressionSearchProblem> ExpressionSearchProblemPtr;

class ExpressionEquivalenceClass : public Object
{
public:
  ExpressionEquivalenceClass(const ExpressionPtr& node)
    {add(node);}
  ExpressionEquivalenceClass() {}

  void add(ExpressionPtr node)
  {
    if (elements.insert(node).second)
    {
      sequences.push_back(LuapeRPNSequence::fromNode(node));
      if (!representent || isSmallerThan(node, representent))
        representent = node;
    }
  }

  size_t getNumElements() const
    {return sequences.size();}

  const LuapeRPNSequencePtr& getSequence(size_t index) const
    {jassert(index < sequences.size()); return sequences[index];}

  const ExpressionPtr& getRepresentent() const
    {return representent;}

  const std::set<ExpressionPtr>& getElements() const
    {return elements;}

  virtual String toShortString() const
    {return T("[") + representent->toShortString() + T("]");}

protected:
  friend class ExpressionEquivalenceClassClass;

  std::set<ExpressionPtr> elements;
  std::vector<LuapeRPNSequencePtr> sequences;
  ExpressionPtr representent;

  bool isSmallerThan(const ExpressionPtr& a, const ExpressionPtr& b) const
  {
    size_t na = a->getTreeSize();
    size_t nb = b->getTreeSize();
    if (na != nb)
      return na < nb;
    int ta = getNodeType(a);
    int tb = getNodeType(b);
    if (ta != tb)
      return ta < tb;
    if (ta == 0)
      return a.staticCast<ConstantExpression>()->getValue() < b.staticCast<ConstantExpression>()->getValue();
    return a->toShortString() < b->toShortString();
  }

  int getNodeType(const ExpressionPtr& node) const
  {
    if (node.isInstanceOf<ConstantExpression>())
      return 0;
    else if (node.isInstanceOf<VariableExpression>())
      return 1;
    else
      return 2;
  }
};

typedef ReferenceCountedObjectPtr<ExpressionEquivalenceClass> ExpressionEquivalenceClassPtr;

class ExpressionEquivalenceClasses : public Object
{
public:
  void add(ExecutionContext& context, const std::vector<ExpressionPtr>& nodes, ExpressionSearchProblemPtr problem, bool verbose = false, BanditPoolPtr pool = BanditPoolPtr())
  {
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      ExpressionPtr node = nodes[i];
      add(context, node, problem->makeBinaryKey(context, node), verbose, pool);
    }
    if (verbose)
    {
      context.informationCallback(String((int)getNumClasses()) + T(" equivalence classes"));
      context.informationCallback(String((int)getNumInvalids()) + T(" invalid candidates"));
    }
  }

  void add(ExecutionContext& context, ExpressionPtr node, BinaryKeyPtr key, bool verbose = false, BanditPoolPtr pool = BanditPoolPtr())
  {
    if (key)
    {
      Map::const_iterator it = m.find(key);
      if (it == m.end())
      {
        ExpressionEquivalenceClassPtr equivalenceClass = new ExpressionEquivalenceClass(node);
        m[key] = equivalenceClass;
        if (pool)
          pool->createArm(equivalenceClass);

        if (verbose && (m.size() % 1000) == 0)
          context.informationCallback(String((int)m.size()) + T(" equivalence classes, last: ") + m[key]->toShortString());
      }
      else
        it->second->add(node);
    }
    else
    {
      if (!invalids)
        invalids = new ExpressionEquivalenceClass(node);
      else
        invalids->add(node);
    }
  }

  size_t getNumClasses() const
    {return m.size();}

  size_t getNumInvalids() const
    {return invalids ? invalids->getNumElements() : 0;}

  void addClassesToBanditPool(BanditPoolPtr pool)
  {
    for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
      pool->createArm(it->second);
  }

protected:
  typedef std::map<BinaryKeyPtr, ExpressionEquivalenceClassPtr, ObjectComparator> Map; // todo: hash_map
  Map m;

  ExpressionEquivalenceClassPtr invalids;
};

typedef ReferenceCountedObjectPtr<ExpressionEquivalenceClasses> ExpressionEquivalenceClassesPtr;

//////////////////////////////////////////////////////////

class LuapeFormulaDiscoverySandBox : public WorkUnit
{
public:
  LuapeFormulaDiscoverySandBox() :
    complexity(5), explorationCoefficient(5.0), numIterations(100), numStepsPerIteration(100), useMultiThreading(false) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
    {
      context.errorCallback(T("Missing problem"));
      return false;
    }

    if (!problem->initializeProblem(context))
      return false;

    ExpressionEquivalenceClassesPtr equivalenceClasses = new ExpressionEquivalenceClasses();
    BanditPoolPtr pool = new BanditPool(new ObjectiveWrapper(problem), explorationCoefficient, false, useMultiThreading); 

    //LuapeRPNSequencePtr subSequence = new LuapeRPNSequence();
    bool cont = true;
    for (size_t iteration = 0; iteration < numIterations && cont; ++iteration)
    {
      context.enterScope(T("Iteration ") + String((int)iteration+1));// + T(", Subsequence: ") + subSequence->toShortString());
      context.resultCallback(T("iteration"), iteration+1);
      cont = doIteration(context, equivalenceClasses, pool);
      context.leaveScope();
    }
    return true;
  }

protected:
  friend class LuapeFormulaDiscoverySandBoxClass;

  ExpressionSearchProblemPtr problem;
  size_t complexity;
  double explorationCoefficient;
  size_t numIterations;
  size_t numStepsPerIteration;
  bool useMultiThreading;

  bool doIteration(ExecutionContext& context, ExpressionEquivalenceClassesPtr equivalenceClasses, BanditPoolPtr pool)
  {
    context.enterScope(T("Enumerating candidates"));
    std::vector<ExpressionPtr> candidates;     
    problem->enumerateNodesExhaustively(context, complexity, candidates, true);
    context.leaveScope(candidates.size());
    context.resultCallback(T("numCandidates"), candidates.size());

    if (candidates.empty())
    {
      context.errorCallback(T("No candidates"));
      return false;
    }
    
    size_t prevNumArms = pool->getNumArms();
    context.enterScope(T("Making equivalence classes"));
    equivalenceClasses->add(context, candidates, problem, true, pool);
    candidates.clear(); // free memory
    context.leaveScope(equivalenceClasses->getNumClasses());
    context.resultCallback(T("numEquivalenceClasses"), equivalenceClasses->getNumClasses());
    context.resultCallback(T("numInvalids"), equivalenceClasses->getNumInvalids());

    if (equivalenceClasses->getNumClasses() == 0)
    {
      context.errorCallback(T("No valid equivalence classes"));
      return false;
    }

    if (pool->getNumArms() > prevNumArms)
    {
      context.enterScope(T("Playing bandits"));
      pool->playIterations(context, numStepsPerIteration, pool->getNumArms() - prevNumArms);
      context.leaveScope();
    }

    size_t bestArm = pool->sampleArmWithHighestReward(context);
    String bestEquivalenceClass = pool->getArmParameter(bestArm).getObjectAndCast<ExpressionEquivalenceClass>()->toShortString();
    context.resultCallback(T("bestArm"), bestEquivalenceClass);
    context.resultCallback(T("bestArmScore"), pool->getArmMeanObjective(bestArm));
    context.resultCallback(T("bestArmPlayedCount"), pool->getArmPlayedCount(bestArm));
    
    context.enterScope(T("Most important nodes"));
    std::multimap<double, ExpressionPtr> nodesByImportance;
    getMostImportantNodes(context, equivalenceClasses, pool, nodesByImportance);
    context.leaveScope();

    const std::set<ExpressionPtr>& actives = problem->getActiveVariables();

    for (std::multimap<double, ExpressionPtr>::reverse_iterator it = nodesByImportance.rbegin(); it != nodesByImportance.rend(); ++it)
    {
      ExpressionPtr node = it->second;
      if (!node.isInstanceOf<VariableExpression>() && !node.isInstanceOf<ConstantExpression>() && actives.find(node) == actives.end())
      {
        context.informationCallback(T("New Active Variable: ") + node->toShortString());
        context.resultCallback(T("newActiveVariable"), node->toShortString());
        problem->addActiveVariable(node);
        return true;
      }
    }

    return false;

#if 0
    LuapeRPNSequencePtr newSubSequence = new LuapeRPNSequence();
    for (size_t i = 0; i < subSequence->getLength(); ++i)
    {
      ObjectPtr bestSymbol = findBestSymbolCompletion(context, pool, newSubSequence);
      if (bestSymbol == subSequence->getElement(i))
        newSubSequence->append(bestSymbol);
      else
        return false;
    }
    ObjectPtr bestSymbol = findBestSymbolCompletion(context, pool, newSubSequence);
    if (bestSymbol)
      newSubSequence->append(bestSymbol);
    else
      return false;

    subSequence = newSubSequence;
#endif // 0
  }

  void getMostImportantNodes(ExecutionContext& context, ExpressionEquivalenceClassesPtr equivalenceClasses, BanditPoolPtr pool, std::multimap<double, ExpressionPtr>& res)
  {
    // clear importances
    ExpressionUniversePtr universe = problem->getUniverse();
    universe->clearImportances();

    // add importances
    std::vector< std::pair<size_t, double> > order;
    pool->getArmsOrder(order);
    for (size_t i = 0; i < order.size() && i < 10; ++i)
    {
      ExpressionEquivalenceClassPtr equivalenceClass = pool->getArmParameter(order[i].first).getObjectAndCast<ExpressionEquivalenceClass>();
      size_t treeSize = equivalenceClass->getRepresentent()->getTreeSize();

      const std::set<ExpressionPtr>& nodes = equivalenceClass->getElements();
      double reward = -order[i].second;
      for (std::set<ExpressionPtr>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        if ((*it)->getTreeSize() == treeSize)
          (*it)->addImportance(reward);
    }

    // display
    std::map<ExpressionPtr, double> importances;
    universe->getImportances(importances);
    ExpressionUniverse::displayMostImportantNodes(context, importances);

    for (std::map<ExpressionPtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
      res.insert(std::make_pair(it->second, it->first));
  }

#if 0
  ObjectPtr findBestSymbolCompletion(ExecutionContext& context, BanditPoolPtr pool, const LuapeRPNSequencePtr& subSequence)
  {
    std::vector< std::pair<size_t, double> > order;
    pool->getArmsOrder(order);

    std::set<ObjectPtr> candidates;
    
    for (size_t i = 0; i < order.size() && candidates.size() != 1; ++i)
    {
      ExpressionEquivalenceClassPtr equivalenceClass = pool->getArmParameter(order[i].first).getObjectAndCast<ExpressionEquivalenceClass>();
      std::set<ObjectPtr> candidateActions = getCandidateActions(equivalenceClass, subSequence);
      /*String info = equivalenceClass->toShortString() + T(" => ");
      for (std::set<ObjectPtr>::const_iterator it = candidateActions.begin(); it != candidateActions.end(); ++it)
        info += (*it)->toShortString() + T(" ");
      context.informationCallback(info);*/

      if (i == 0)
      {
        candidates = candidateActions;
        if (candidates.empty())
          return ObjectPtr();
      }
      else
      {
        std::set<ObjectPtr> remainingCandidates;
        std::set_intersection(candidateActions.begin(), candidateActions.end(), candidates.begin(), candidates.end(),
                                std::inserter(remainingCandidates, remainingCandidates.begin()));
        if (remainingCandidates.empty())
          break;
        candidates.swap(remainingCandidates);
      }
      
      /*info = String::empty;
      for (std::set<ObjectPtr>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
        info += (*it)->toShortString() + T(" ");
      context.informationCallback(info);*/
    }

    jassert(candidates.size());
    String info("Final candidates: ");
    for (std::set<ObjectPtr>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
      info += (*it)->toShortString() + T(" ");
    context.informationCallback(info);

    if (candidates.size() == 1)
      return *candidates.begin();

    size_t n = context.getRandomGenerator()->sampleSize(candidates.size());
    std::set<ObjectPtr>::const_iterator it = candidates.begin();
    for (size_t i = 0; i < n; ++i)
      ++it;
    return *it;
  }

  std::set<ObjectPtr> getCandidateActions(ExpressionEquivalenceClassPtr equivalenceClass, const LuapeRPNSequencePtr& subSequence)
  {
    size_t n = equivalenceClass->getNumElements();
    std::set<ObjectPtr> res;
    for (size_t i = 0; i < n; ++i)
    {
      LuapeRPNSequencePtr sequence = equivalenceClass->getSequence(i);
      if (sequence->startsWith(subSequence) && sequence->getLength() > subSequence->getLength())
        res.insert(sequence->getElement(subSequence->getLength()));
    }
    return res;
  }
#endif // 0

  struct ObjectiveWrapper : public BanditPoolObjective
  {
    ObjectiveWrapper(ExpressionSearchProblemPtr problem) : problem(problem) {}

    ExpressionSearchProblemPtr problem;

    virtual size_t getNumInstances() const
      {return problem->getNumInstances();}

    virtual void getObjectiveRange(double& worst, double& best) const
      {problem->getObjectiveRange(worst, best);}
 
    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      ExpressionEquivalenceClassPtr equivalenceClass = parameter.getObjectAndCast<ExpressionEquivalenceClass>();
      return problem->computeObjective(context, equivalenceClass->getRepresentent(), instanceIndex);
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_H_
