/*-----------------------------------------.---------------------------------.
| Filename: Choose.cpp                     | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 20:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/CRAlgorithm.h>
#include <lbcpp/FeatureGenerator.h>
#include <lbcpp/impl/impl.h>
using namespace lbcpp;

class ChooseActionValueFunction : public ActionValueFunction
{
public:
   virtual void setChoose(ChoosePtr choose)
     {function = choose->getActionValueFunction();}

   virtual double compute(VariablePtr choice) const
     {return function->compute(choice);}

private:
  ActionValueFunctionPtr function;     
};

ActionValueFunctionPtr ActionValueFunction::createChooseActionValue()
  {return new ChooseActionValueFunction();}

StateValueFunctionPtr StateValueFunction::createPredictions(RegressorPtr regressor)
  {return impl::staticToDynamic(impl::RegressorStateValueFunction(regressor));}

StateValueFunctionPtr StateValueFunction::createPredictions(RankerPtr ranker)
  {return impl::staticToDynamic(impl::RankerStateValueFunction(ranker));}

ActionValueFunctionPtr ActionValueFunction::createScores(ClassifierPtr classifier)
  {return impl::staticToDynamic(impl::ClassifierScoresActionValue(classifier));}

ActionValueFunctionPtr ActionValueFunction::createScores(GeneralizedClassifierPtr classifier)
  {return impl::staticToDynamic(impl::GeneralizedClassifierScoresActionValue(classifier));}

ActionValueFunctionPtr ActionValueFunction::createPredictions(RankerPtr ranker)
  {return impl::staticToDynamic(impl::RankerActionValueFunction(ranker));}

ActionValueFunctionPtr ActionValueFunction::createProbabilities(ClassifierPtr classifier)
  {return impl::staticToDynamic(impl::ClassifierProbabilitiesActionValue(classifier));}

ActionValueFunctionPtr ActionValueFunction::createProbabilities(GeneralizedClassifierPtr classifier)
  {return impl::staticToDynamic(impl::GeneralizedClassifierProbabilitiesActionValue(classifier));}

ActionValueFunctionPtr ActionValueFunction::createPredictions(RegressorPtr regressor)
  {return impl::staticToDynamic(impl::RegressorActionValueFunction(regressor));}

/*
** Choose
*/
std::string Choose::computeStateDescription() const
{
  StateDescriptionFunctionPtr f = getStateDescriptionFunction();
  if (f)
    return f->compute();
  else
  {
    warning("Choose::computeStateDescription", "Missing state description");
    return "";
  }
}

std::string Choose::computeActionDescription(VariablePtr choice) const
{
  ActionDescriptionFunctionPtr f = getActionDescriptionFunction();
  if (f)
    return f->compute(choice);
  else
  {
    warning("Choose::computeActionDescription", "Missing action description");
    return "";
  }
}

double Choose::computeStateValue() const
{
  StateValueFunctionPtr f = getStateValueFunction();
  if (f)
    return f->compute();
  else
  {
    warning("Choose::stateValue", "Missing state value");
    return 0.0;
  }
}

double Choose::computeActionValue(VariablePtr choice) const
{
  ActionValueFunctionPtr f = getActionValueFunction();
  if (f)
    return f->compute(choice);
  else
  {
    warning("Choose::actionValue", "Missing action value");
    return 0.0;
  }
}
  
FeatureGeneratorPtr Choose::computeStateFeatures() const
{
  StateFeaturesFunctionPtr f = getStateFeaturesFunction();
  if (f)
    return f->compute();
  else
  {
    warning("Choose::computeStateFeatures", "Missing state feature generator");
    return FeatureGeneratorPtr();
  }
}
  
FeatureGeneratorPtr Choose::computeActionFeatures(VariablePtr choice) const
{
  assert(choice);
  ActionFeaturesFunctionPtr f = getActionFeaturesFunction();
  if (f)
    return f->compute(choice);
  else
  {
    warning("Choose::actionFeatures", "Missing action feature generator");
    return FeatureGeneratorPtr();
  }
}

/*
** toString
*/
std::string Choose::toString() const
{
  std::string res = crAlgorithm->toString();
  res += "\n";
  std::vector<ActionValueFunctionPtr> actionValues(getNumActionValues());
  for (size_t i = 0; i < actionValues.size(); ++i)
  {
    ActionValueFunctionPtr f = getActionValue(i);
    f->setChoose(getReferenceCountedPointer());
    actionValues[i] = f;
  }
  for (VariableIteratorPtr iterator = newIterator(); iterator->exists(); iterator->next())
  {
    VariablePtr choice = iterator->get();
    res += "Choice " + choice->toString();
    for (size_t i = 0; i < actionValues.size(); ++i)
    {
      ActionValueFunctionPtr f = actionValues[i];
      res += " " + f->getName() + ": " + lbcpp::toString(f->compute(choice));
    }
    res += "\n";
  }
  return res;
}
