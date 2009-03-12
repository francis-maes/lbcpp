/*-----------------------------------------.---------------------------------.
| Filename: Choose.cpp                     | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 20:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/CRAlgorithm.h>
using namespace cralgo;
  
std::string Choose::stateDescription() const
{
  StateDescriptionFunctionPtr f = getStateDescriptionFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute();
}

std::string Choose::actionDescription(VariablePtr choice) const
{
  ActionDescriptionFunctionPtr f = getActionDescriptionFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute(choice);
}

double Choose::stateValue() const
{
  StateValueFunctionPtr f = getStateValueFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute();
}

double Choose::actionValue(VariablePtr choice) const
{
  ActionValueFunctionPtr f = getActionValueFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute(choice);
}
  
FeatureGeneratorPtr Choose::stateFeatures() const
{
  StateFeaturesFunctionPtr f = getStateFeaturesFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute();
}
  
FeatureGeneratorPtr Choose::actionFeatures(VariablePtr choice) const
{
  ActionFeaturesFunctionPtr f = getActionFeaturesFunction();
  f->setChoose(getReferenceCountedPointer());
  return f->compute(choice);
}
