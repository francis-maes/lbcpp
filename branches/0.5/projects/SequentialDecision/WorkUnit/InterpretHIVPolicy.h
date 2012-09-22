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
# include "../GP/PolicyFormulaSearchProblem.h"

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

class ClassifierBasedPolicy : public Policy
{
public:
  ClassifierBasedPolicy(const LuapeClassifierPtr& classifier = LuapeClassifierPtr())
    : classifier(classifier) {}

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    ContainerPtr actions = state->getAvailableActions();
    DenseDoubleVectorPtr probabilities = classifier->compute(context, state, Variable()).getObjectAndCast<DenseDoubleVector>();
    int label = probabilities->getIndexOfMaximumValue();
    jassert(label >= 0);
    return label >= 0 ? actions->getElement(label) : Variable();
  }

  virtual String toShortString() const
    {return classifier->getRootNode()->toShortString();}

protected:
  friend class ClassifierBasedPolicyClass;

  LuapeClassifierPtr classifier;
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

class ComputeDecisionProblemSuccessorState : public LuapeFunction
{
public:
  ComputeDecisionProblemSuccessorState(DecisionProblemPtr problem = DecisionProblemPtr())
    : problem(problem) {}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return index == 0 ? type->inheritsFrom(decisionProblemStateClass) : (!problem || type == problem->getActionType());}

  virtual TypePtr initialize(const TypePtr* inputTypes)
  {
    outputClass = pairClass(inputTypes[0], doubleType);
    return outputClass;
  }
    
  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return T("succ(") + inputs[0]->toShortString() + T(", ") + inputs[1]->toShortString() + T(")");}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const DecisionProblemStatePtr& state = inputs[0].getObjectAndCast<DecisionProblemState>();
    Variable action = getAction(inputs);
    double reward;
    DecisionProblemStatePtr res = state->cloneAndCast<DecisionProblemState>();
    res->performTransition(context, action, reward);
    return new Pair(outputClass, res, reward);
  }

protected:
  friend class ComputeDecisionProblemSuccessorStateClass;

  DecisionProblemPtr problem;

  ClassPtr outputClass;
  
  virtual Variable getAction(const Variable* inputs) const
    {return inputs[1];}
};

class ComputeDecisionProblemSuccessorStateFunctor : public ComputeDecisionProblemSuccessorState
{
public:
  ComputeDecisionProblemSuccessorStateFunctor(const DecisionProblemPtr& problem = DecisionProblemPtr())
    : ComputeDecisionProblemSuccessorState(problem), action(0) {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return T("succ(") + inputs[0]->toShortString() + T(", ") + String((int)action) + T(")");}

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    size_t numActions = problem->getFixedNumberOfActions();
    jassert(numActions);
    VectorPtr res = vector(positiveIntegerType, numActions);
    for (size_t i = 0; i < numActions; ++i)
      res->setElement(i, i);
    return res;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ComputeDecisionProblemSuccessorStateFunctorClass;
        
  size_t action;

  virtual Variable getAction(const Variable* inputs) const
  {
    const DecisionProblemStatePtr& state = inputs[0].getObjectAndCast<DecisionProblemState>();
    DecisionProblemStatePtr res = state->cloneAndCast<DecisionProblemState>();
    ContainerPtr availableActions = state->getAvailableActions();
    jassert(action < availableActions->getNumElements());
    return availableActions->getElement(action);
  }
};


class CostSensitiveMultiClassAccuracyObjective : public ClassificationLearningObjective
{
public:
  virtual void initialize(const LuapeInferencePtr& problem)
  {
    ClassificationLearningObjective::initialize(problem);
    for (size_t i = 0; i < 3; ++i)
      costs[i] = new DenseDoubleVector(doubleVectorClass);
  }

  virtual void setSupervisions(const VectorPtr& supervisions)
  {
    jassert(supervisions->getElementsType() == denseDoubleVectorClass(labels, doubleType));
    this->supervisions = supervisions.staticCast<ObjectVector>();
  }

  virtual void update()
  {
    for (size_t i = 0; i < 3; ++i)
      costs[i]->multiplyByScalar(0.0);
    
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      size_t example = it.getIndex();
      unsigned char prediction = it.getRawBoolean();

      const DenseDoubleVectorPtr& supervisionCosts = supervisions->get(example).staticCast<DenseDoubleVector>();
      supervisionCosts->addTo(costs[prediction]);
    }
  }

  virtual void flipPrediction(size_t index)
  {
    jassert(upToDate);
    const DenseDoubleVectorPtr& supervisionCosts = supervisions->get(index).staticCast<DenseDoubleVector>();
    supervisionCosts->subtractFrom(costs[0]);
    supervisionCosts->addTo(costs[1]);
  }

  virtual double computeObjective() // sum of costs, if each sub-tree was a constant vote
  {
    ensureIsUpToDate();
    return -(costs[0]->getMinimumValue() + costs[1]->getMinimumValue() + costs[2]->getMinimumValue());
  }

  virtual Variable computeVote(const IndexSetPtr& indices)
  {
    if (indices->size() == 0)
      return Variable::missingValue(denseDoubleVectorClass(labels, probabilityType));
    DenseDoubleVectorPtr costs = new DenseDoubleVector(doubleVectorClass);
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    {
      const DenseDoubleVectorPtr& supervisionCosts = supervisions->get(*it).staticCast<DenseDoubleVector>();
      supervisionCosts->addTo(costs);
    }
    DenseDoubleVectorPtr res = new DenseDoubleVector(labels, probabilityType);
    res->setValue(costs->getIndexOfMinimumValue(), 1.0);
    return res;
  }

protected:
  friend class CostSensitiveMultiClassAccuracyClass;

  ObjectVectorPtr supervisions;
  DenseDoubleVectorPtr costs[3];
};

extern EnumerationPtr hivActionEnumeration;

class InterpretHIVPolicy : public WorkUnit
{
public:
  InterpretHIVPolicy() : numInitialStates(100), maxHorizon(300), rolloutLength(50), randomActionProbability(0.0), discount(0.98) {}

  virtual Variable run(ExecutionContext& context)
  {
    DecisionProblemPtr problem = hivDecisionProblem();
    ContainerPtr initialStates = problem->sampleInitialStates(context, numInitialStates);

    ContainerPtr examples;

    if (dataFile.exists())
    {
      examples = loadDataFile(context, dataFile);
    }
    else
    {
      PolicyPtr referencePolicy = createReferencePolicy(context, problem);
      ScalarVariableStatisticsPtr returnStatistics = new ScalarVariableStatistics("return");

      examples = createLearningExamples(context, problem, referencePolicy, initialStates, returnStatistics);
    }

#if 0
    // E / log(T1)
    GPExpressionPtr formula1 = new BinaryGPExpression(new VariableGPExpression(Variable(5, hivStateVariablesEnumeration)),
                                                     gpDivision,
                                                     new UnaryGPExpression(gpLog, new VariableGPExpression(Variable(0, hivStateVariablesEnumeration))));

    testDiscoveredPolicy(context, initialStates, new FormulaBasedHIVPolicy(formula1));

    // sqrt(E) / log(T1)
    GPExpressionPtr formula2 = new BinaryGPExpression(new UnaryGPExpression(gpSquareRoot, new VariableGPExpression(Variable(5, hivStateVariablesEnumeration))),
                                                     gpDivision,
                                                     new UnaryGPExpression(gpLog, new VariableGPExpression(Variable(0, hivStateVariablesEnumeration))));
    testDiscoveredPolicy(context, initialStates, new FormulaBasedHIVPolicy(formula2));

    // E
    GPExpressionPtr formula3 = new VariableGPExpression(Variable(5, hivStateVariablesEnumeration));
    testDiscoveredPolicy(context, initialStates, new FormulaBasedHIVPolicy(formula3));
#endif // 0
    return true;


    PolicyPtr interpretablePolicy = findInterpretablePolicy(context, initialStates, examples);

    testDiscoveredPolicy(context, initialStates, interpretablePolicy);
    return true;
  }

protected:
  friend class InterpretHIVPolicyClass;

  size_t numInitialStates;
  size_t maxHorizon;
  size_t rolloutLength;
  double randomActionProbability;
  double discount;
  File dataFile;

  PolicyPtr createReferencePolicy(ExecutionContext& context, DecisionProblemPtr problem)
  {
    FunctionPtr featuresFunction = new SimpleSearchNodeFeatureGenerator(true, true);
    if (!featuresFunction->initialize(context, searchTreeNodeClass(problem->getStateClass(), problem->getActionType())))
      return PolicyPtr();
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
    jassert(parameters->getNumValues() == (size_t)tokens.size());
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

  ContainerPtr createLearningExamples(ExecutionContext& context, const DecisionProblemPtr& problem, const PolicyPtr& policy, const ContainerPtr& initialStates, ScalarVariableStatisticsPtr returnStatistics)
  {
    File outputFile = context.getFile("egreedy-samples" + String((int)maxHorizon) + T("_") + String((int)numInitialStates) + T("_") + String((int)rolloutLength) + T(".txt"));
    if (outputFile.existsAsFile())
      outputFile.deleteFile();
    OutputStream* ostr = outputFile.createOutputStream();

    TypePtr examplesClass = pairClass(hivDecisionProblemStateClass, probabilityType);
    ObjectVectorPtr examples = new ObjectVector(examplesClass, 0);

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
        // select action
        Variable action = policy->selectAction(context, state);
        DenseDoubleVectorPtr actionVector = action.getObjectAndCast<DenseDoubleVector>();


        DenseDoubleVectorPtr regrets;
        if (rolloutLength > 0)
          regrets = computeActionRegrets(context, problem, state, policy);
        else
        {
          regrets = new DenseDoubleVector(hivActionEnumeration, doubleType, (size_t)-1, 1.0);
          if (actionVector->getValue(0) == 0.0)
          {
            if (actionVector->getValue(1) == 0.0)
              regrets->setValue(0, 0.0);
            else
              regrets->setValue(2, 0.0);
          }
          else
          {
            if (actionVector->getValue(1) == 0.0)
              regrets->setValue(1, 0.0);
            else
              regrets->setValue(3, 0.0);
          }
        }

        // write to file
        std::vector<double> stateValues;
        hivState->getState(stateValues);
        String line;
        for (size_t j = 0; j < stateValues.size(); ++j)
          line += String(stateValues[j]) + T(" ");
        for (size_t j = 0; j < regrets->getNumValues(); ++j)
          line += String(regrets->getValue(j)) + T(" ");
        line += T("\n");
        *ostr << line;
        ostr->flush();

        {
          context.resultCallback(T("a1"), actionVector->getValue(0));
          context.resultCallback(T("a2"), actionVector->getValue(1));
        }

        // save to memory
        examples->append(new Pair(examplesClass, state->cloneAndCast<DecisionProblemState>(), regrets));

        // take a random action with randomActionProbability
        if (context.getRandomGenerator()->sampleBool(randomActionProbability))
        {
          ContainerPtr actions = state->getAvailableActions();
          action = actions->getElement(context.getRandomGenerator()->sampleSize(actions->getNumElements()));
        }
     
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
    context.informationCallback(String((int)examples->getNumElements()) + T(" examples"));

    delete ostr;
    return examples;
  }

  ContainerPtr loadDataFile(ExecutionContext& context, const File& dataFile)
  {
    InputStream* istr = dataFile.createInputStream();
    if (!istr)
    {
      context.errorCallback(T("Could not open file ") + dataFile.getFileName());
      return ContainerPtr();
    }
    ClassPtr examplesClass = pairClass(hivDecisionProblemStateClass, denseDoubleVectorClass(hivActionEnumeration, doubleType));
    ObjectVectorPtr res = new ObjectVector(examplesClass);

    while (!istr->isExhausted())
    {
      String line = istr->readNextLine();
      StringArray tokens;
      tokens.addTokens(line, true);
      if (tokens.size() == 0)
        continue;

      jassert(tokens.size() >= 10);
      std::vector<double> stateValues(6);
      for (size_t i = 0; i < stateValues.size(); ++i)
        stateValues[i] = tokens[i].getDoubleValue();
      HIVDecisionProblemStatePtr state = new HIVDecisionProblemState(stateValues);
      
      DenseDoubleVectorPtr costs = new DenseDoubleVector(hivActionEnumeration, doubleType);
      for (size_t i = 0; i < costs->getNumValues(); ++i)
        costs->setValue(i, tokens[i + 6].getDoubleValue());

      res->append(new Pair(examplesClass, state, costs));
    }

    delete istr;
    return res;
  }

  PolicyPtr findInterpretablePolicy(ExecutionContext& context, const ContainerPtr& initialStates, const ContainerPtr& examples)
  {
    context.enterScope(T("Searching interpretable policy"));
    size_t n = examples->getNumElements();
    context.informationCallback(String((int)n) + T(" examples"));
    std::vector<size_t> countPerClass(4, 0);
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr supervision = examples->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<DenseDoubleVector>();
      int label = supervision->getIndexOfMinimumValue();
      jassert(label >= 0 && label < 4);
      countPerClass[label]++;
    }
    String info;
    for (size_t i = 0; i < countPerClass.size(); ++i)
    {
      info += String((int)countPerClass[i]) + T(" ") + hivActionEnumeration->getElementName(i);
      if (i < countPerClass.size() - 1)
        info += T(", ");
    }
    context.informationCallback(info);

    LuapeClassifierPtr bestClassifier;
    double bestObjectiveValue = DBL_MAX;
    for (size_t complexity = 3; complexity <= 18; complexity += 3)
      for (size_t treeDepth = 3; treeDepth <= 3; ++treeDepth)
      {
        context.enterScope(T("complexity = ") + String((int)complexity) + T(" treeDepth = ") + String((int)treeDepth));
        context.resultCallback(T("complexity"), complexity);
        context.resultCallback(T("treeDepth"), treeDepth);

        LuapeClassifierPtr classifier = new LuapeClassifier();
        classifier->initialize(context, hivDecisionProblemStateClass, denseDoubleVectorClass(hivActionEnumeration, doubleType));

        classifier->addInput(hivDecisionProblemStateClass, "s");
        classifier->addFunction(andBooleanLuapeFunction());
        classifier->addFunction(equalBooleanLuapeFunction());
        classifier->addFunction(addDoubleLuapeFunction());
        classifier->addFunction(subDoubleLuapeFunction());
        classifier->addFunction(mulDoubleLuapeFunction());
        classifier->addFunction(divDoubleLuapeFunction());
        //classifier->addFunction(logDoubleLuapeFunction());
        classifier->addFunction(greaterThanDoubleLuapeFunction());
        classifier->addFunction(getVariableLuapeFunction());
        //classifier->addFunction(new ComputeDecisionProblemSuccessorStateFunctor(decisionProblem));
        classifier->setSamples(context, examples.staticCast<ObjectVector>()->getObjects());
        classifier->getTrainingCache()->setMaxSizeInMegaBytes(512);
        LuapeNodeBuilderPtr nodeBuilder = exhaustiveSequentialNodeBuilder(complexity);
        nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
        
        LuapeLearnerPtr learner = optimizerBasedSequentialWeakLearner(new NestedMonteCarloOptimizer(3, 1), complexity);
        
        learner->setVerbose(true);

        //LearningObjectivePtr objective = new CostSensitiveMultiClassAccuracyObjective();
        LearningObjectivePtr objective = new InformationGainLearningObjective(true);
        objective->initialize(classifier);
        learner->setObjective(objective);
        learner = treeLearner(objective, learner, 2, treeDepth);
      
        LuapeNodePtr node = learner->learn(context, LuapeNodePtr(), classifier, classifier->getTrainingCache()->getAllIndices());
        classifier->setRootNode(context, node);
        double error = classifier->evaluatePredictions(context, classifier->getTrainingPredictions(), classifier->getTrainingSupervisions());
        double policyReturn = 0.0;
        if (node)
        {
          context.informationCallback(node->toShortString());
          if (error < bestObjectiveValue)
          {
            bestObjectiveValue = error;
            bestClassifier = classifier;
          }
          policyReturn = testDiscoveredPolicy(context, initialStates, new ClassifierBasedPolicy(classifier));
        }
        context.resultCallback(T("error"), error);
        context.resultCallback(T("policyReturn"), policyReturn);
        if (node)
          context.resultCallback(T("node"), node);

        //classifier->setLearner(weakLearner, true);

        //classifier->setLearner(adaBoostLearner(weakLearner, 10, 1), true); // 10 iterations, tree depth = 1

        //ScoreObjectPtr score = classifier->train(context, examples, ContainerPtr(), T("Training"), true);
        context.leaveScope(error);
      }

    context.leaveScope(bestObjectiveValue);
    return new ClassifierBasedPolicy(bestClassifier);
  }

  double testDiscoveredPolicy(ExecutionContext& context, const ContainerPtr& initialStates, const PolicyPtr& policy)
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
    return returnStatistics->getMean();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_INTERPRET_HIV_POLICY_H_
