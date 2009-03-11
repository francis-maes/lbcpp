/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExampleCrea...hpp| A policy that creates           |
| Author  : Francis Maes                   |   classification examples       |
| Started : 11/03/2009 21:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_
# define CRALGO_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_

# include "PolicyStatic.hpp"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct ClassificationExampleCreatorPolicy
  : public DecoratorPolicy<ClassificationExampleCreatorPolicy<DecoratedType> , DecoratedType>
{
  typedef ClassificationExampleCreatorPolicy<DecoratedType> ExactType;
  typedef DecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  ClassificationExampleCreatorPolicy(const DecoratedType& explorationPolicy, ClassifierPtr classifier, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : BaseClass(explorationPolicy), classifier(classifier), supervisor(supervisor) {}
    
  ClassifierPtr classifier;
  ActionValueFunctionPtr supervisor;

  CRAlgorithmPtr crAlgorithm;
  
  void policyEnter(CRAlgorithmPtr crAlgo)
  {
    // check flat
    assert(!crAlgorithm); crAlgorithm = crAlgo;
    
    classifier->trainStochasticBegin();
    BaseClass::policyEnter(crAlgo);
  }
  
  const void* policyChoose(ChoosePtr choose)
  {
    // this policy can only be used with size_t choices
    if (choose->getChoiceType() == "size_t")
    {
      size_t output = *(size_t* )choose->sampleBestChoice(supervisor ? supervisor : choose->getActionValueFunction());
      classifier->trainStochasticExample(ClassificationExample(choose->stateFeatures(), output));
    }
    else
      ErrorHandler::error("ClassificationExampleCreatorPolicy::policyChoose", 
          "This policy can only be used choices of type 'size_t'");
    return BaseClass::policyChoose(choose);
  }
  
  void policyLeave()
  {
    crAlgorithm = CRAlgorithmPtr();
    BaseClass::policyLeave();
    classifier->trainStochasticEnd();
  }
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_
