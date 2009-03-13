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
  : public EpisodicDecoratorPolicy<ClassificationExampleCreatorPolicy<DecoratedType> , DecoratedType>
{
  typedef ClassificationExampleCreatorPolicy<DecoratedType> ExactType;
  typedef EpisodicDecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  ClassificationExampleCreatorPolicy(const DecoratedType& explorationPolicy, ClassifierPtr classifier, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : BaseClass(explorationPolicy), classifier(classifier), supervisor(supervisor) {}
    
  ClassifierPtr classifier;
  ActionValueFunctionPtr supervisor;

  void episodeEnter(CRAlgorithmPtr crAlgo)
    {classifier->trainStochasticBegin();}
  
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
      classifier->trainStochasticExample(ClassificationExample(stateFeatures, bestChoice->getReference<size_t>()));
    }
    else
      BaseClass::error("ClassificationExampleCreatorPolicy::policyChoose", 
          "This policy can only be used choices of type 'size_t'");
    return BaseClass::policyChoose(choose);
  }
  
  void episodeLeave()
    {classifier->trainStochasticEnd();}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_CLASSIFICATION_EXAMPLE_CREATOR_POLICY_H_
