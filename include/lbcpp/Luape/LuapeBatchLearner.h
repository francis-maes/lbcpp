/*-----------------------------------------.---------------------------------.
| Filename: LuapeBatchLearner.h            | Luape Batch Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_H_
# define LBCPP_LUAPE_BATCH_LEARNER_H_

# include "LuapeLearner.h"
# include "LuapeCache.h"
# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class LuapeBatchLearner : public BatchLearner
{
public:
  LuapeBatchLearner(LuapeLearnerPtr learner, size_t maxIterations)
    : learner(learner), maxIterations(maxIterations), plotOutputStream(NULL) {}
  LuapeBatchLearner() {}
  virtual ~LuapeBatchLearner()
  {
    if (plotOutputStream)
      delete plotOutputStream;
  }

  virtual TypePtr getRequiredFunctionType() const
    {return luapeInferenceClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeInferencePtr& function = f.staticCast<LuapeInference>();

    LuapeLearnerPtr learner = this->learner->cloneAndCast<LuapeLearner>(); // avoid cycle between LuapeInference -> LuapeBatchLearner -> LuapeLearner -> LuapeInference
    if (!learner->initialize(context, function))
      return false;

    learner->setExamples(context, true, trainingData);
    if (validationData.size())
      learner->setExamples(context, false, validationData);

    context.enterScope(T("Boosting"));

    if (plotOutputStream)
    {
      *plotOutputStream << "# " << String((int)trainingData.size()) << " training examples, " << String((int)validationData.size()) << " validation examples\n";
      *plotOutputStream << "# Learner: " << learner->toShortString() << "\n";
      *plotOutputStream << "# Iterations: " << String((int)maxIterations) << "\n\n";
      plotOutputStream->flush();
    }

    context.informationCallback(String((int)trainingData.size()) + T(" training examples"));
    if (validationData.size())
      context.informationCallback(String((int)validationData.size()) + T(" validation examples"));

    LuapeUniversePtr universe = function->getUniverse();

    ScalarVariableMean lastIterationsValidationScore;
    double bestValidationScore = DBL_MAX;
    for (size_t i = 0; i < maxIterations; ++i)
    {
      //learner->getTrainingCache()->displayCacheInformation(context);
      //Object::displayObjectAllocationInfo(std::cout);

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
      
      double trainingScore, validationScore;
      learner->doLearningIteration(context, trainingScore, validationScore);
      if (learner->getVerbose())
      {
        context.resultCallback("trainCacheSizeInMb", learner->getTrainingCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
        if (learner->getValidationCache())
          context.resultCallback("validationCacheSizeInMb", learner->getValidationCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
      }
      context.resultCallback(T("log10(iteration)"), log10((double)i+1.0));

      if (validationScore < bestValidationScore)
        bestValidationScore = validationScore;
      if (plotOutputStream)
      {
        *plotOutputStream << String((int)i+1) << " " << String(trainingScore) << " " << String(validationScore) << " " << String(bestValidationScore) << "\n";
        plotOutputStream->flush();
      }
      if (i >= 4 * maxIterations / 5)
        lastIterationsValidationScore.push(validationScore);

      context.leaveScope();

      //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    
      if (learner->getVerbose())
	context.enterScope(T("Most important nodes"));
      displayMostImportantNodes(context, function, learner->getVerbose());
      if (learner->getVerbose())
	context.leaveScope();
    }
    context.leaveScope();
  
    context.enterScope(T("Most important nodes"));
    displayMostImportantNodes(context, function, true);
    context.leaveScope();

    context.informationCallback(T("Best evaluation: ") + String(bestValidationScore * 100.0, 3) + T("%"));
    context.informationCallback(T("Last 20% iteration evaluation: ") + String(lastIterationsValidationScore.getMean() * 100, 3) + T("%"));
    if (plotOutputStream)
    {
      *plotOutputStream << "\n# best evaluation score: " << String(bestValidationScore * 100.0, 3) << "%\n";
      *plotOutputStream << "# last 20% iteration evaluation: " << String(lastIterationsValidationScore.getMean() * 100, 3) << "%\n\n";
      plotOutputStream->flush();
    }
      
    //Object::displayObjectAllocationInfo(std::cerr);
    //context.resultCallback("votes", function->getVotes());
    return true;
  }
  
  void getImportances(const LuapeNodePtr& node, std::map<LuapeNodePtr, double>& res) const
  {
    if (node && res.find(node) == res.end())
    {
      double importance = node->getImportance();
      jassert(isNumberValid(importance));
      if (importance > 0)
      //if (!node.isInstanceOf<LuapeFunctionNode>() || node.staticCast<LuapeFunctionNode>()->getFunction()->getClassName() != T("StumpLuapeFunction"))
        res[node] = importance;
      //node->setImportance(0.0);
      size_t n = node->getNumSubNodes();
      for (size_t i = 0; i < n; ++i)
        getImportances(node->getSubNode(i), res);
    }
  }
  
  void displayMostImportantNodes(ExecutionContext& context, const LuapeInferencePtr& function, bool verbose) const
  {
    const LuapeNodePtr& rootNode = function->getRootNode();

    // get importance values
    std::map<LuapeNodePtr, double> importances;
    getImportances(rootNode, importances);

    // create probabilities and nodes vectors
    double Z = 0.0;
    std::vector<double> probabilities(importances.size());
    std::vector<LuapeNodePtr> nodes(importances.size());
    size_t index = 0;
    for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it, ++index)
    {
      Z += it->second;
      probabilities[index] = it->second;
      nodes[index] = it->first;
    }

    // display most important nodes
    if (verbose)
    {
      std::multimap<double, LuapeNodePtr> nodeImportanceMap;
      for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
        nodeImportanceMap.insert(std::make_pair(it->second, it->first));
      size_t i = 0;
      for (std::multimap<double, LuapeNodePtr>::reverse_iterator it = nodeImportanceMap.rbegin(); it != nodeImportanceMap.rend() && i < 20; ++it, ++i)
      {
        if (it->first <= 0.0)
          break;
        const LuapeNodePtr& node = it->second;
        context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + node->toShortString() + T(" [") + String(it->first * 100.0 / Z, 2) + T("%]"));
      }
    }

    // sample new active variables
    function->clearActiveVariables();
    while (function->getNumActiveVariables() < 10 && Z > 1e-12)
    {
      jassert(isNumberValid(Z));
      size_t index = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
      LuapeNodePtr node = nodes[index];
      if (!node.isInstanceOf<LuapeInputNode>())
      {
        context.informationCallback(T("Active variable: ") + node->toShortString());
        function->addActiveVariable(node);
      }
      Z -= probabilities[index];
      probabilities[index] = 0.0;
    }
  }

  void setPlotFile(ExecutionContext& context, const File& plotFile)
  {
    jassert(!plotOutputStream);
    if (plotFile.existsAsFile())
      plotFile.deleteFile();
    plotOutputStream = plotFile.createOutputStream();
    if (!plotOutputStream)
      context.warningCallback(T("Could not create file ") + plotFile.getFullPathName());
  }

  OutputStream* getPlotOutputStream() const
    {return plotOutputStream;}

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr learner;
  size_t maxIterations;

  OutputStream* plotOutputStream;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
