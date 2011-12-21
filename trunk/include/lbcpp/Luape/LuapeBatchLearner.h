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

    LuapeNodeUniversePtr universe = function->getUniverse();

    ScalarVariableMean lastIterationsValidationScore;
    double bestValidationScore = DBL_MAX;
    for (size_t i = 0; i < maxIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
      
      double trainingScore, validationScore;
      learner->doLearningIteration(context, trainingScore, validationScore);
      if (learner->getVerbose())
      {
        context.resultCallback("trainCacheSizeInMb", learner->getTrainingCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
        if (learner->getValidationCache())
          context.resultCallback("validationCacheSizeInMb", learner->getValidationCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
        //learner->getTrainingCache()->displayCacheInformation(context);
        //       learner->getTrainingCache()->getComputeTimeStatistics(context);
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
      
    /*  if ((i+1) % 10 == 0)
      {
        context.enterScope(T("Most important nodes"));
        displayMostImportantNodes(context, function);
        context.leaveScope();
      }*/
    }
    context.leaveScope();

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
    if (node)
    {
      res[node] = node->getImportance();
      size_t n = node->getNumSubNodes();
      for (size_t i = 0; i < n; ++i)
        getImportances(node->getSubNode(i), res);
    }
  }
  
  void displayMostImportantNodes(ExecutionContext& context, const LuapeInferencePtr& function) const
  {
    const LuapeNodePtr& rootNode = function->getRootNode();

    std::map<LuapeNodePtr, double> importances;
    getImportances(rootNode, importances);
    
    std::multimap<double, LuapeNodePtr> nodeImportanceMap;
    double importanceSum = 0.0;
    for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
    {
      nodeImportanceMap.insert(std::make_pair(it->second, it->first));
      importanceSum += it->second;
    }
    
    size_t i = 0;
    function->clearActiveVariables();
    for (std::multimap<double, LuapeNodePtr>::reverse_iterator it = nodeImportanceMap.rbegin(); it != nodeImportanceMap.rend(); ++it, ++i)
    {
      const LuapeNodePtr& node = it->second;
      if (i < 20)
        context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + node->toShortString() + T(" [") + String(it->first * 100.0 / importanceSum, 2) + T("%]"));

      /// !!! TEST
      /// !!!
      if (!node.isInstanceOf<LuapeInputNode>())
      {
        function->addActiveVariable(node);
        if (function->getNumActiveVariables() >= 20)
          break;
      }
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
