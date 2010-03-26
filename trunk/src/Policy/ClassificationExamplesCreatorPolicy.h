/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExamplesCreat...h| A policy that creates           |
| Author  : Francis Maes                   |   classification examples       |
| Started : 11/03/2009 21:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_CLASSIFICATION_EXAMPLES_CREATOR_H_
# define LBCPP_POLICY_CLASSIFICATION_EXAMPLES_CREATOR_H_

# include <lbcpp/Policy.h>
# include <lbcpp/LearningMachine.h>

namespace lbcpp
{

class ClassificationExamplesCreatorPolicy : public DecoratorPolicy
{
public:  
  ClassificationExamplesCreatorPolicy(PolicyPtr explorationPolicy, ClassifierPtr classifier, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : DecoratorPolicy(explorationPolicy), classifier(classifier), supervisor(supervisor), inclusionLevel(0), isTraining(false) {}
  ClassificationExamplesCreatorPolicy() {}
    
  /*
  ** Object
  */
  virtual String toString() const
  {
    String res = "classificationExamplesCreatorPolicy(" + decorated->toString() + ", " + classifier->toString();
    if (supervisor)
      res += ", " + supervisor->toString();
    return res + ")";
  }
  
  virtual void save(std::ostream& ostr) const
    {DecoratorPolicy::save(ostr); write(ostr, classifier); write(ostr, supervisor);}
    
  virtual bool load(std::istream& istr)
    {return DecoratorPolicy::load(istr) && read(istr, classifier) && read(istr, supervisor);}
  
  /*
  ** Policy
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (inclusionLevel == 0)
      isTraining = true;
    DecoratorPolicy::policyEnter(crAlgorithm);
    ++inclusionLevel;
  }
    
  virtual void policyLeave()
  {
    DecoratorPolicy::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0 && isTraining)
    {
      classifier->trainStochasticEnd();
      isTraining = false;
    }
  }

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    // this policy can only be used with size_t choices
    if (choose->getChoiceType() == T("size_t"))
    {
      FeatureGeneratorPtr stateFeatures = choose->computeStateFeatures();
      jassert(stateFeatures);
//      std::cout << "State Features: " << stateFeatures->toString() << std::endl;
      VariablePtr bestChoice = choose->sampleBestChoice(supervisor ? supervisor : choose->getActionValueFunction());
//      std::cout << "Best Choice: " << bestChoice->getReference<size_t>() << std::endl;
      jassert(bestChoice);
      if (!isTraining)
      {
        classifier->trainStochasticBegin(stateFeatures->getDictionary());
        isTraining = true;
      }
      classifier->trainStochasticExample(new ClassificationExample(stateFeatures, bestChoice->getReference<size_t>()));
    }
    else
      Object::error("ClassificationExampleCreatorPolicy::policyChoose", 
          "This policy can only be used choices of type 'size_t'");
    return DecoratorPolicy::policyChoose(choose);
  }
  
protected:
  ClassifierPtr classifier;
  ActionValueFunctionPtr supervisor;
  size_t inclusionLevel;
  bool isTraining;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_CLASSIFICATION_EXAMPLES_CREATOR_H_
