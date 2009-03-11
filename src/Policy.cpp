/*-----------------------------------------.---------------------------------.
| Filename: Policy.cpp                     | Policies                        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

inline impl::DynamicToStaticPolicy dynamicToStatic(const Policy* policy)
  {return impl::dynamicToStatic(PolicyPtr(const_cast<Policy* >(policy)));}

inline impl::DynamicToStaticPolicy dynamicToStatic(const PolicyPtr policy)
  {return impl::dynamicToStatic(policy);}

// 

PolicyPtr Policy::createRandom()
  {return impl::staticToDynamic(impl::RandomPolicy());}

PolicyPtr Policy::createGreedy(ActionValueFunctionPtr actionValues)
  {return impl::staticToDynamic(impl::GreedyPolicy(actionValues));}

PolicyPtr Policy::createClassificationExampleCreator(PolicyPtr explorationPolicy,
          ClassifierPtr classifier, ActionValueFunctionPtr supervisor)
{
  return impl::staticToDynamic(impl::ClassificationExampleCreatorPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(explorationPolicy), classifier, supervisor));
}

PolicyPtr Policy::epsilonGreedy(IterationFunctionPtr epsilon) const
  {return impl::staticToDynamic(impl::EpsilonGreedyPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this), epsilon));}
  
PolicyPtr Policy::addComputeStatistics() const
  {return impl::staticToDynamic(impl::ComputeStatisticsPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this)));}
