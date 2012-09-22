/*-----------------------------------------.---------------------------------.
| Filename: DSBFormulaSearchProblem.h      | DSB Formula Search Problem      |
| Author  : Francis Maes                   |                                 |
| Started : 25/04/2012 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_DSB_FORMULA_SEARCH_PROBLEM_H_
# define LBCPP_PROTEINS_LUAPE_DSB_FORMULA_SEARCH_PROBLEM_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "LuapeProteinPredictorParameters.h"
# include "../Predictor/ProteinPredictor.h"
# include "../Evaluator/ProteinEvaluator.h"
# include "../Evaluator/KolmogorovPerfectMatchingFunction.h"
# include "../../SequentialDecision/Luape/LuapeFormulaDiscovery.h"

namespace lbcpp
{

class DSBFormulaSearchProblem : public LuapeNodeSearchProblem
{
public:
  virtual bool initializeProblem(ExecutionContext& context)
  {
    trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    testingProteins = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (trainingProteins.empty() || testingProteins.empty())
      return false;

    // input
    addInput(proteinResiduePairPerceptionClass, "bond");

    // constants
    addConstant(1.0);
    addConstant(2.0);
    addConstant(3.0);
    addConstant(5.0);
    addConstant(7.0);

    // boolean operations
    addFunction(andBooleanLuapeFunction());
    addFunction(equalBooleanLuapeFunction());

    // integer operations
    addFunction(addIntegerLuapeFunction());
    addFunction(subIntegerLuapeFunction());
    addFunction(mulIntegerLuapeFunction());
    addFunction(divIntegerLuapeFunction());

    // unary double operations
    addFunction(logDoubleLuapeFunction());
    addFunction(absDoubleLuapeFunction());
    addFunction(oppositeDoubleLuapeFunction());
    addFunction(inverseDoubleLuapeFunction());

    // binary double operators
    addFunction(addDoubleLuapeFunction());
    addFunction(subDoubleLuapeFunction());
    addFunction(mulDoubleLuapeFunction());
    addFunction(divDoubleLuapeFunction());

    // enumeration operations
    addFunction(equalsConstantEnumLuapeFunction());

    // object operations
    addFunction(getVariableLuapeFunction());
    addFunction(getContainerLengthLuapeFunction());

    // double vectors
    addFunction(getDoubleVectorElementLuapeFunction());
    addFunction(computeDoubleVectorStatisticsLuapeFunction());
    addFunction(getDoubleVectorExtremumsLuapeFunction());

    // protein-specific operations
    addFunction(new ProteinGetRelativeResidueLuapeFunction());

    evaluator = new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(-1.f), 0.f);
    //evaluator = new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0);
    return true;
  }

  virtual size_t getNumInstances() const
    {return trainingProteins.size();}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  double getPrediction(const Variable& value) const
  {
    double res = value.toDouble();
    return res == doubleMissingValue || !isNumberValid(res) ? 0.0 : res;
  }

  virtual double computeObjective(ExecutionContext& context, const LuapeNodePtr& node, size_t instanceIndex)
  {
    instanceIndex %= trainingProteins.size();

    // retrieve supervision
    SymmetricMatrixPtr supervision = trainingProteins[instanceIndex].second->getDisulfideBonds(context);
    size_t n = supervision->getNumRows();

    // make raw predictions
    LuapeSamplesCachePtr cache = makeCache(trainingProteins[instanceIndex].first);
    VectorPtr predictions = cache->getSamples(context, node)->getVector();
    if (!predictions)
      return 0.0;

    // compute min value and max value
    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;
    for (size_t index = 0; index < predictions->getNumElements(); ++index)
    {
      double value = getPrediction(predictions->getElement(index));
      if (value < minValue)
        minValue = value;
      if (value > maxValue)
        maxValue = value;
    }

    // turn into symmetric matrix of probabilities
    DoubleSymmetricMatrixPtr prediction = new DoubleSymmetricMatrix(probabilityType, n, 0.0);
    size_t index = 0;
    for (size_t j = 0; j < n; ++j)
      for (size_t k = j + 1; k < n; ++k)
      {
        jassert(index < predictions->getNumElements());
        double value = getPrediction(predictions->getElement(index++));
        double probability = 1.0 / (1.0 + exp(-value / 100.0));//maxValue > minValue ? (value - minValue) / (maxValue - minValue) : 0.5;
        prediction->setValue(j, k, probability);
      }
    jassert(index == n * (n - 1) / 2);

    // evaluate
    ScoreObjectPtr score = evaluator->createEmptyScoreObject(context, FunctionPtr());
    evaluator->addPrediction(context, prediction, supervision, score);
    evaluator->finalizeScoreObject(score, FunctionPtr());
    return 1.0 - score->getScoreToMinimize();
  }

  bool useInput(const LuapeNodePtr& node) const
  {
    if (node.dynamicCast<LuapeInputNode>())
      return true;
    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      if (useInput(node->getSubNode(i)))
        return true;
    return false;
  }

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const LuapeNodePtr& node) const
  {
    if (!useInput(node))
      return BinaryKeyPtr();

    std::vector<double> pred;
    ScalarVariableStatistics predStats;
    size_t count = 30;
    if (trainingProteins.size() < count)
      count = trainingProteins.size();
    for (size_t i = 0; i < count; ++i)
    {
      LuapeSamplesCachePtr cache = makeCache(trainingProteins[i].first);
      VectorPtr predictions = cache->getSamples(context, node)->getVector();
      for (size_t j = 0; j < predictions->getNumElements(); ++j)
      {
        double activation = getPrediction(predictions->getElement(j));
        predStats.push(activation);
        pred.push_back(activation);
      }
    }
   
    double minValue = predStats.getMinimum();
    double maxValue = predStats.getMaximum();

    BinaryKeyPtr key = new BinaryKey(4 * pred.size());
    for (size_t i = 0; i < pred.size(); ++i)
    {
      double value = maxValue > minValue ? (pred[i] - minValue) / (maxValue - minValue) : 0.5;
      key->push32BitInteger((int)(10000 * value));
    }
    return key;
  }

protected:
  friend class DSBFormulaSearchProblemClass;

  File trainingInputDirectory;
  File trainingSupervisionDirectory;
  File testingInputDirectory;
  File testingSupervisionDirectory;
  size_t maxProteinCount;

  std::vector< std::pair< ProteinPerceptionPtr, ProteinPtr> > trainingProteins;
  std::vector< std::pair< ProteinPerceptionPtr, ProteinPtr> > testingProteins;
  SupervisedEvaluatorPtr evaluator;

  std::vector< std::pair< ProteinPerceptionPtr, ProteinPtr> >  loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    size_t n = proteins ? proteins->getNumElements() : 0;
    context.informationCallback(String((int)n) + T(" ") + description + T(" proteins"));

    std::vector< std::pair< ProteinPerceptionPtr, ProteinPtr> > res(n);
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr pair = proteins->getElement(i).getObject();
      res[i].first = new ProteinPerception(pair->getVariable(0).getObjectAndCast<Protein>(), false);
      res[i].second = pair->getVariable(1).getObjectAndCast<Protein>();
    }
    return res;
  }

  LuapeSamplesCachePtr makeCache(const ProteinPerceptionPtr& perception) const
  {
    const std::vector<size_t>& cysteinIndices = perception->getCysteinIndices();
    size_t n = perception->getNumCysteins();
    
    LuapeSamplesCachePtr res = createCache(n * (n - 1) / 2, 512);
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j)
      {
        ObjectPtr residuePair = new ProteinResiduePairPerception(perception, perception->getResidue(cysteinIndices[i]), perception->getResidue(cysteinIndices[j]));
        res->setInputObject(inputs, index++, residuePair);
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_DSB_FORMULA_SEARCH_PROBLEM_H_
