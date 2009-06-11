/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExampleCrea...hpp| A policy that creates           |
| Author  : Francis Maes                   |   classification examples       |
| Started : 11/03/2009 21:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_
# define LBCPP_CORE_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_

# include "PolicyStatic.hpp"
# include "../../LearningMachine.h"

namespace lbcpp {
namespace impl {

template<class DecoratedType>
struct ClassificationExampleCreatorPolicy
  : public DecoratorPolicy<ClassificationExampleCreatorPolicy<DecoratedType> , DecoratedType>
{
  typedef ClassificationExampleCreatorPolicy<DecoratedType> ExactType;
  typedef DecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  ClassificationExampleCreatorPolicy(const DecoratedType& explorationPolicy, ClassifierPtr classifier, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : BaseClass(explorationPolicy), classifier(classifier), supervisor(supervisor), inclusionLevel(0), isTraining(false) {}
    
  ClassifierPtr classifier;
  ActionValueFunctionPtr supervisor;
  size_t inclusionLevel;
  bool isTraining;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (inclusionLevel == 0)
      isTraining = true;
    BaseClass::policyEnter(crAlgorithm);
    ++inclusionLevel;
  }
    
  void policyLeave()
  {
    BaseClass::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0 && isTraining)
    {
      classifier->trainStochasticEnd();
      isTraining = false;
    }
  }

  VariablePtr policyChoose(ChoosePtr choose)
  {
    // this policy can only be used with size_t choices
    if (choose->getChoiceType() == "size_t")
    {
      FeatureGeneratorPtr stateFeatures = choose->computeStateFeatures();
      assert(stateFeatures);
//      std::cout << "State Features: " << stateFeatures->toString() << std::endl;
      VariablePtr bestChoice = choose->sampleBestChoice(supervisor ? supervisor : choose->getActionValueFunction());
//      std::cout << "Best Choice: " << bestChoice->getReference<size_t>() << std::endl;
      assert(bestChoice);
      if (!isTraining)
      {
        classifier->trainStochasticBegin(stateFeatures->getDictionary());
        isTraining = true;
      }
      classifier->trainStochasticExample(new ClassificationExample(stateFeatures, bestChoice->getReference<size_t>()));
    }
    else
      BaseClass::error("ClassificationExampleCreatorPolicy::policyChoose", 
          "This policy can only be used choices of type 'size_t'");
    return BaseClass::policyChoose(choose);
  }
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_
