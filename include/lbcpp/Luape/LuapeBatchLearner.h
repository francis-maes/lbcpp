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
    : learner(learner), maxIterations(maxIterations) {}
  LuapeBatchLearner() {}

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

    context.informationCallback(String((int)trainingData.size()) + T(" training examples"));
    if (validationData.size())
      context.informationCallback(String((int)validationData.size()) + T(" validation examples"));

    LuapeNodeUniversePtr universe = function->getUniverse();
    for (size_t i = 0; i < maxIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
      context.resultCallback(T("log10(iteration)"), log10((double)i+1.0));
      
      learner->doLearningIteration(context);
      if (learner->getVerbose())
      {
        context.resultCallback("trainCacheSizeInMb", learner->getTrainingCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
        if (learner->getValidationCache())
          context.resultCallback("validationCacheSizeInMb", learner->getValidationCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
        //learner->getTrainingCache()->displayCacheInformation(context);
        //       learner->getTrainingCache()->getComputeTimeStatistics(context);
      }
      context.leaveScope();

      //if ((i+1) % 100 == 0)
      //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
      
   //   if ((i + 1) % 100 == 0)
   //     analyseGraph(context, function->getRootNode());
    }
    context.leaveScope();
    //Object::displayObjectAllocationInfo(std::cerr);
    //context.resultCallback("votes", function->getVotes());
    return true;
  }
  
  void fillUsageStats(const LuapeNodePtr& node, std::map<LuapeNodePtr, double>& res) const
  {
    if (node)
    {
      res[node] += 1.0;
      size_t n = node->getNumSubNodes();
      for (size_t i = 0; i < n; ++i)
        fillUsageStats(node->getSubNode(i), res);
    }
  }
  
  void analyseGraph(ExecutionContext& context, const LuapeNodePtr& rootNode) const
  {
    std::map<LuapeNodePtr, double> usageStats;
    fillUsageStats(rootNode, usageStats);
    
    std::multimap<double, LuapeNodePtr> nodesByUsage;
    for (std::map<LuapeNodePtr, double>::const_iterator it = usageStats.begin(); it != usageStats.end(); ++it)
      nodesByUsage.insert(std::make_pair(it->second, it->first));
    
    size_t i = 0;
    for (std::multimap<double, LuapeNodePtr>::reverse_iterator it = nodesByUsage.rbegin(); it != nodesByUsage.rend() && i < 100; ++it, ++i)
      context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + it->second->toShortString() + T(" [") + String(it->first) + T("]"));
  }

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr learner;
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
