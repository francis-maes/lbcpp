/*-----------------------------------------.---------------------------------.
| Filename: InterpretHIVPolicy.h           | Interpret HIV Policy            |
| Author  : Francis Maes                   |                                 |
| Started : 16/01/2012 20:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_INTERPRET_HIV_POLICY_H_
# define LBCPP_SEQUENTIAL_DECISION_INTERPRET_HIV_POLICY_H_

# include "HIVSandBox.h"
# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/DecisionProblem/Policy.h>
# include "../Core/NestedMonteCarloOptimizer.h"

namespace lbcpp
{

class LookAHeadPolicy : public Policy
{
public:
  LookAHeadPolicy(FunctionPtr searchHeuristic, size_t budget)
    : searchHeuristic(searchHeuristic), budget(budget), searchPolicy(new BestFirstSearchPolicy(searchHeuristic))
  {
  }
  LookAHeadPolicy() : budget(0) {}

  virtual void startEpisode(ExecutionContext& context, const DecisionProblemPtr& problem, const DecisionProblemStatePtr& initialState)
    {this->problem = problem;}

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    SearchTreePtr searchTree = new SearchTree(problem, state, budget);
    searchTree->doSearchEpisode(context, searchPolicy, budget);
    return searchTree->getBestAction();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LookAHeadPolicyClass;

  FunctionPtr searchHeuristic;
  size_t budget;

  SearchPolicyPtr searchPolicy;
  DecisionProblemPtr problem;
};

class TwoBinaryClassifiersHIVPolicy : public Policy
{
public:
  TwoBinaryClassifiersHIVPolicy(const LuapeBinaryClassifierPtr& a1, const LuapeBinaryClassifierPtr& a2)
    : a1(a1), a2(a2) {}
  TwoBinaryClassifiersHIVPolicy() {}

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    Variable missingSupervision = Variable::missingValue(probabilityType);
    DenseDoubleVectorPtr action = new DenseDoubleVector(2, 0.0);
    action->setValue(0, a1->compute(context, state, missingSupervision).getDouble() * 0.7);
    action->setValue(1, a2->compute(context, state, missingSupervision).getDouble() * 0.3);
    return action;
  }

  virtual String toShortString() const
    {return a1->getRootNode()->toShortString() + T("; ") + a2->getRootNode()->toShortString();}

protected:
  friend class TwoBinaryClassifiersHIVPolicyClass;

  LuapeBinaryClassifierPtr a1;
  LuapeBinaryClassifierPtr a2;
};

class InterpretHIVPolicy : public WorkUnit
{
public:
  InterpretHIVPolicy() : numInitialStates(100), maxHorizon(300), rolloutLength(50), discount(0.98) {}

  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    DecisionProblemPtr problem = hivDecisionProblem(discount);
    ContainerPtr initialStates = problem->sampleInitialStates(context, random, numInitialStates);

    PolicyPtr referencePolicy = createReferencePolicy(context, problem);
    ScalarVariableStatisticsPtr returnStatistics = new ScalarVariableStatistics("return");

    std::pair<ContainerPtr, ContainerPtr> examples = createLearningExamples(context, problem, referencePolicy, initialStates, returnStatistics);
    LuapeBinaryClassifierPtr a1 = findBestFormula(context, "a1", examples.first);
    LuapeBinaryClassifierPtr a2 = findBestFormula(context, "a2", examples.second);
    PolicyPtr interpretablePolicy = new TwoBinaryClassifiersHIVPolicy(a1, a2);

    testDiscoveredPolicy(context, initialStates, interpretablePolicy);
    return true;
  }


protected:
  friend class InterpretHIVPolicyClass;

  size_t numInitialStates;
  size_t maxHorizon;
  size_t rolloutLength;
  double discount;

  PolicyPtr createReferencePolicy(ExecutionContext& context, DecisionProblemPtr problem)
  {
    FunctionPtr featuresFunction = new SimpleSearchNodeFeatureGenerator(true, true);
    if (!featuresFunction->initialize(context, searchTreeNodeClass(problem->getStateClass(), problem->getActionType())))
      return false;
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(featuresFunction->getOutputType());
    /*
    -0.104502031 -0.337083007 1.45883731 -0.989125727 0.916016545 -1.19195962 1.29469848 -0.704655225 -0.908633602 -0.253788533 1.43174049 -0.750602368 -0.734515027 -0.827228593 -0.974346984 1.19547833 -1.25078751 0.184333038
-1.6370195 1.39123935 0.0636134093 1.6959735 0.405791341 0.669316748 -1.14016202 0.627656638 0.19573995 -0.58317319 -0.129234267 -0.299858431 0.220057346 1.20208049 0.52486993 -0.186980508 1.29633586 0.622683974
-1.60499496 0.106143119 -0.0266703773 0.134817162 -0.293141034 3.05116625 -1.76633516 -0.359784686 -1.15641967 -1.91776103 -1.53437465 -0.990104771 0.726565312 0.639334682 -3.44724385 -1.86257693 -1.20610861 -0.265995966
-0.299284102 0.0616315405 0.441865364 -0.384043143 -0.570205839 0.539600637 -1.78039608 -0.335913275 -1.50946733 -0.188811147 -1.69003162 -0.571616793 -0.721855367 -0.445439127 0.611914326 -0.382390317 -1.39627892 0.588879568
-0.0361650256 -0.996838137 -0.512097762 0.42789346 -0.219663704 0.55420503 -1.48208362 -0.593661834 -1.81698709 0.230869851 -2.81577272 -0.120334515 -0.0665108357 -0.629174489 -0.56849791 -0.597812452 0.219920589 -1.47121247
0.894523602 1.19929614 0.993329871 -0.0677094665 0.696950242 3.15939044 -0.630735484 -0.0164870716 -0.399100524 0.609717444 -1.05809382 0.0425629618 0.494840122 -0.110886859 -1.23804899 -0.80566502 -0.624816906 -1.05789091
    */

    size_t budget = 85;
    String line("-0.299284102 0.0616315405 0.441865364 -0.384043143 -0.570205839 0.539600637 -1.78039608 -0.335913275 -1.50946733 -0.188811147 -1.69003162 -0.571616793 -0.721855367 -0.445439127 0.611914326 -0.382390317 -1.39627892 0.588879568");
    StringArray tokens;
    tokens.addTokens(line, true);

    DenseDoubleVectorPtr parameters = new DenseDoubleVector(featuresEnumeration, doubleType);
    jassert(parameters->getNumValues() == tokens.size());
    for (int i = 0; i < tokens.size(); ++i)
      parameters->setValue(i, tokens[i].getDoubleValue());
    FunctionPtr searchHeuristic = new HIVSearchHeuristic(featuresFunction, parameters);
    return new LookAHeadPolicy(searchHeuristic, budget);
  }

  DenseDoubleVectorPtr computeActionRegrets(ExecutionContext& context, const DecisionProblemPtr& problem, const DecisionProblemStatePtr& state, const PolicyPtr& policy)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    double maxCumulativeReward = -DBL_MAX;

    for (size_t i = 0; i < n; ++i)
    {
      DecisionProblemStatePtr stateCopy = state->cloneAndCast<DecisionProblemState>();
      Variable action = actions->getElement(i);
      double cumulativeReward;
      stateCopy->performTransition(context, action, cumulativeReward);
      policy->startEpisode(context, problem, stateCopy);
      for (size_t j = 0; j < rolloutLength; ++j)
      {
        Variable action = policy->selectAction(context, stateCopy);
        double reward;
        stateCopy->performTransition(context, action, reward);
        cumulativeReward += reward * pow(discount, j + 1.0);
      }
      
      res->setValue(i, cumulativeReward);
      if (cumulativeReward > maxCumulativeReward)
        maxCumulativeReward = cumulativeReward;
    }
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, maxCumulativeReward - res->getValue(i));
    return res;
  }

  std::pair<ContainerPtr, ContainerPtr> createLearningExamples(ExecutionContext& context, const DecisionProblemPtr& problem, const PolicyPtr& policy, const ContainerPtr& initialStates, ScalarVariableStatisticsPtr returnStatistics)
  {
    File outputFile = context.getFile("wsamples" + String((int)maxHorizon) + T("_") + String((int)numInitialStates) + T(".txt"));
    OutputStream* ostr = outputFile.createOutputStream();

    TypePtr examplesClass = pairClass(hivDecisionProblemStateClass, probabilityType);
    ObjectVectorPtr a1Examples = new ObjectVector(examplesClass, 0);
    ObjectVectorPtr a2Examples = new ObjectVector(examplesClass, 0);

    context.enterScope(T("Creating learning examples"));
    for (size_t i = 0; i < initialStates->getNumElements(); ++i)
    {
      context.enterScope(T("Trajectory ") + String((int)i + 1));
      DecisionProblemStatePtr state = initialStates->getElement(i).clone(context).getObjectAndCast<DecisionProblemState>();
      jassert(state);
      policy->startEpisode(context, problem, state);
      double cumulativeReward = 0.0;
      for (size_t t = 0; t < maxHorizon; ++t)
      {
        context.enterScope(T("Step ") + String((int)t + 1));
        context.resultCallback(T("step"), t);
        HIVDecisionProblemStatePtr hivState = state.staticCast<HIVDecisionProblemState>();
        hivState->addAsResults(context);


        // write to file
        std::vector<double> stateValues;
        hivState->getState(stateValues);
        String line;
        for (size_t j = 0; j < stateValues.size(); ++j)
          line += String(stateValues[j]) + T(" ");
        DenseDoubleVectorPtr regrets = computeActionRegrets(context, problem, state, policy);
        for (size_t j = 0; j < regrets->getNumValues(); ++j)
          line += String(regrets->getValue(j)) + T(" ");
        line += T("\n");
        *ostr << line;
        ostr->flush();

        // select action
        Variable action = policy->selectAction(context, state);
        DenseDoubleVectorPtr actionVector = action.getObjectAndCast<DenseDoubleVector>();
        
        {
          context.resultCallback(T("a1"), actionVector->getValue(0));
          context.resultCallback(T("a2"), actionVector->getValue(1));
        }

        // save to memory
        DecisionProblemStatePtr stateCopy = state->cloneAndCast<DecisionProblemState>();
        bool a1 = actionVector->getValue(0) > 0.0;
        bool a2 = actionVector->getValue(1) > 0.0;
        a1Examples->append(new Pair(examplesClass, stateCopy, a1 ? 1.0 : 0.0));
        a2Examples->append(new Pair(examplesClass, stateCopy, a2 ? 1.0 : 0.0));        

        // perform transition
        double reward = 0.0;
        state->performTransition(context, action, reward);
        cumulativeReward += reward * pow(discount, (double)t);
        context.resultCallback(T("reward"), reward);
        if (cumulativeReward > 0)
          context.resultCallback(T("log return"), log10(cumulativeReward));

        context.leaveScope(reward);
        context.progressCallback(new ProgressionState(t+1, maxHorizon, T("Steps")));
      }
      returnStatistics->push(cumulativeReward);
      context.leaveScope(cumulativeReward);
      context.progressCallback(new ProgressionState(i+1, initialStates->getNumElements(), T("Trajectories")));
    }
    context.leaveScope(returnStatistics->getMean());

    delete ostr;
    return std::make_pair(a1Examples, a2Examples);
  }

  LuapeBinaryClassifierPtr findBestFormula(ExecutionContext& context, const String& name, const ContainerPtr& examples)
  {
    context.enterScope(T("Searching best formula for ") + name);
    size_t n = examples->getNumElements();
    context.informationCallback(String((int)n) + T(" examples"));
    size_t numPositives = 0;
    for (size_t i = 0; i < n; ++i)
      if (examples->getElement(i).getObjectAndCast<Pair>()->getSecond().getDouble() > 0.0)
        ++numPositives;
    context.informationCallback(String((int)numPositives) + T(" positives, ") + String((int)n - (int)numPositives) + T(" negatives"));

    LuapeBinaryClassifierPtr bestClassifier;
    double bestObjectiveValue = -DBL_MAX;
    for (size_t complexity = 3; complexity <= 6; complexity += 3)
    {
      context.enterScope(T("complexity = ") + String((int)complexity));
      context.resultCallback(T("complexity"), complexity);

      LuapeBinaryClassifierPtr classifier = new LuapeBinaryClassifier();

      classifier->addInput(hivDecisionProblemStateClass, "s");
      classifier->addFunction(andBooleanLuapeFunction());
      classifier->addFunction(equalBooleanLuapeFunction());
      classifier->addFunction(addDoubleLuapeFunction());
      classifier->addFunction(subDoubleLuapeFunction());
      classifier->addFunction(mulDoubleLuapeFunction());
      classifier->addFunction(divDoubleLuapeFunction());
      classifier->addFunction(logDoubleLuapeFunction());
      classifier->addFunction(getVariableLuapeFunction());
      // todo: compute-successor function
      classifier->setSamples(context, examples.staticCast<ObjectVector>()->getObjects());

      LuapeNodeBuilderPtr nodeBuilder = exhaustiveSequentialNodeBuilder(complexity);
      nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
      
      LuapeLearnerPtr learner = optimizerBasedSequentialWeakLearner(new NestedMonteCarloOptimizer(5, 1), complexity);
      //LuapeLearnerPtr learner = exactWeakLearner(nodeBuilder);
      
      learner->setVerbose(true);

      LearningObjectivePtr objective = new BinaryClassificationLearningObjective();
      objective->initialize(classifier);
      learner->setObjective(objective);

      //learner = treeLearner(objective, learner, 2, 5);
    
      LuapeNodePtr node = learner->learn(context, LuapeNodePtr(), classifier, classifier->getTrainingCache()->getAllIndices());
      classifier->setRootNode(context, node);
      double accuracy = 1.0 - classifier->evaluatePredictions(context, classifier->getTrainingPredictions(), classifier->getTrainingSupervisions());
      if (node)
      {
        context.informationCallback(node->toShortString());
        context.resultCallback(T("node"), node);
        if (accuracy > bestObjectiveValue)
        {
          bestObjectiveValue = accuracy;
          bestClassifier = classifier;
        }
      }

      //classifier->setLearner(weakLearner, true);

      //classifier->setLearner(adaBoostLearner(weakLearner, 10, 1), true); // 10 iterations, tree depth = 1

      //ScoreObjectPtr score = classifier->train(context, examples, ContainerPtr(), T("Training"), true);
      context.leaveScope(accuracy);
    }

    context.leaveScope(bestObjectiveValue);
    return bestClassifier;
  }

  void testDiscoveredPolicy(ExecutionContext& context, const ContainerPtr& initialStates, const PolicyPtr& policy)
  {
    context.enterScope(T("Testing policy ") + policy->toShortString());

    ScalarVariableStatisticsPtr returnStatistics = new ScalarVariableStatistics("return");
    for (size_t i = 0; i < initialStates->getNumElements(); ++i)
    {
      context.enterScope(T("Trajectory ") + String((int)i + 1));
      DecisionProblemStatePtr state = initialStates->getElement(i).clone(context).getObjectAndCast<DecisionProblemState>();
      jassert(state);
      double cumulativeReward = 0.0;
      for (size_t t = 0; t < maxHorizon; ++t)
      {
        context.enterScope(T("Step ") + String((int)t + 1));
        context.resultCallback(T("step"), t);
        state.staticCast<HIVDecisionProblemState>()->addAsResults(context);

        DenseDoubleVectorPtr action = policy->selectAction(context, state).getObjectAndCast<DenseDoubleVector>();

        context.resultCallback(T("a1"), action->getValue(0));
        context.resultCallback(T("a2"), action->getValue(1));

        double reward = 0.0;
        state->performTransition(context, action, reward);
        cumulativeReward += reward * pow(discount, (double)t);
        context.resultCallback(T("reward"), reward);
        if (cumulativeReward > 0)
          context.resultCallback(T("log return"), log10(cumulativeReward));

        context.leaveScope(reward);
        context.progressCallback(new ProgressionState(t+1, maxHorizon, T("Steps")));
      }
      returnStatistics->push(cumulativeReward);
      context.leaveScope(cumulativeReward);
      context.progressCallback(new ProgressionState(i+1, initialStates->getNumElements(), T("Trajectories")));
    }
    context.leaveScope(returnStatistics->getMean());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_INTERPRET_HIV_POLICY_H_
