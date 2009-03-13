/*-----------------------------------------.---------------------------------.
| Filename: Choose.cpp                     | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 20:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/CRAlgorithm.h>
#include <cralgo/FeatureGenerator.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

ActionValueFunctionPtr ActionValueFunction::createClassifierScores(ClassifierPtr classifier)
{
  return impl::staticToDynamic(impl::ClassifierScoresActionValue(classifier));
}

std::string Choose::stateDescription() const
{
  StateDescriptionFunctionPtr f = getStateDescriptionFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute();
  }
  else
  {
    warning("Choose::stateDescription", "Missing state description");
    return "";
  }
}

std::string Choose::actionDescription(VariablePtr choice) const
{
  ActionDescriptionFunctionPtr f = getActionDescriptionFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute(choice);
  }
  else
  {
    warning("Choose::actionDescription", "Missing action description");
    return "";
  }
}

double Choose::stateValue() const
{
  StateValueFunctionPtr f = getStateValueFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute();
  }
  else
  {
    warning("Choose::stateValue", "Missing state value");
    return 0.0;
  }
}

double Choose::actionValue(VariablePtr choice) const
{
  ActionValueFunctionPtr f = getActionValueFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute(choice);
  }
  else
  {
    warning("Choose::actionValue", "Missing action value");
    return 0.0;
  }
}
  
FeatureGeneratorPtr Choose::stateFeatures() const
{
  StateFeaturesFunctionPtr f = getStateFeaturesFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute();
  }
  else
  {
    warning("Choose::stateFeatures", "Missing state feature generator");
    return FeatureGeneratorPtr();
  }
}
  
FeatureGeneratorPtr Choose::actionFeatures(VariablePtr choice) const
{
  ActionFeaturesFunctionPtr f = getActionFeaturesFunction();
  if (f)
  {
    f->setChoose(getReferenceCountedPointer());
    return f->compute(choice);
  }
  else
  {
    warning("Choose::actionFeatures", "Missing action feature generator");
    return FeatureGeneratorPtr();
  }
}
