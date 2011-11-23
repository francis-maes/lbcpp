/*-----------------------------------------.---------------------------------.
| Filename: EvaluateSmallMDPPolicies.h     | Evaluate Small MDP Policies     |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2011 20:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_EVALUATE_POLICIES_H_
# define LBCPP_SMALL_MDP_EVALUATE_POLICIES_H_

# include "SmallMDPSandBox.h"

namespace lbcpp
{

class EvaluateSmallMDPPolicies : public WorkUnit
{
public:
  EvaluateSmallMDPPolicies() : mdpSampler(new SparseSmallMDPSampler()), numRuns(1000) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> policyFiles;
    inputDirectory.findChildFiles(policyFiles, File::findFiles, false, "*.policy");

    std::vector< std::pair<String, SmallMDPPolicyPtr> > policies;
    policies.reserve(policyFiles.size());
    for (int i = 0; i < policyFiles.size(); ++i)
    {
      SmallMDPPolicyPtr policy = SmallMDPPolicy::createFromFile(context, *policyFiles[i]);
      if (policy)
        policies.push_back(std::make_pair(policyFiles[i]->getFileNameWithoutExtension(), policy));
    }

    for (size_t i = 0; i < policies.size(); ++i)
      evaluatePolicy(context, policies[i].first, policies[i].second);
    return true;
  }

  double evaluatePolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy) const
  {
    context.enterScope(name);
    context.resultCallback("policy", policy);
    
    CompositeWorkUnitPtr workUnit = new EvaluateSmallMDPPolicyCompositeWorkUnit(policy, mdpSampler, numRuns);
    VariableVectorPtr results = context.run(workUnit, false).getObjectAndCast<VariableVector>();

    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("toto");
    for (size_t i = 0; i < numRuns; ++i)
      stats->push(results->getElement(i).getDouble());
    context.resultCallback("mean cumulative reward", stats->getMean());
    context.leaveScope(stats);
    return stats->getMean();
  }

protected:
  friend class EvaluateSmallMDPPoliciesClass;
  
  File inputDirectory;
  SamplerPtr mdpSampler;
  size_t numRuns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

