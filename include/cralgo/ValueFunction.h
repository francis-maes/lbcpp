/*-----------------------------------------.---------------------------------.
| Filename: ValueFunction.h                | Value functions                 |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 17:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_VALUE_FUNCTION_H_
# define CRALGO_VALUE_FUNCTION_H_

# include "Object.h"

namespace cralgo
{

/*
** ChooseFunction base class
*/
class ChooseFunction : public Object
{
public:
  virtual void setChoose(ChoosePtr choose) = 0;
};
typedef ReferenceCountedObjectPtr<ChooseFunction> ChooseFunctionPtr;

/*
** Values
*/
class StateValueFunction : public ChooseFunction
{
public:
  virtual double compute() const = 0;
};
typedef ReferenceCountedObjectPtr<StateValueFunction> StateValueFunctionPtr;

class ActionValueFunction : public ChooseFunction
{
public:
  virtual double compute(VariablePtr choice) const = 0;
};
typedef ReferenceCountedObjectPtr<ActionValueFunction> ActionValueFunctionPtr;

/*
** Features
*/
class StateFeaturesFunction : public ChooseFunction
{
public:
  virtual FeatureGeneratorPtr compute() const = 0;
};
typedef ReferenceCountedObjectPtr<StateFeaturesFunction> StateFeaturesFunctionPtr;

class ActionFeaturesFunction : public ChooseFunction
{
public:
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const = 0;
};
typedef ReferenceCountedObjectPtr<ActionFeaturesFunction> ActionFeaturesFunctionPtr;

/*
** String Descriptions
*/
class StateDescriptionFunction : public ChooseFunction
{
public:
  virtual std::string compute() const = 0;
};
typedef ReferenceCountedObjectPtr<StateDescriptionFunction> StateDescriptionFunctionPtr;

class ActionDescriptionFunction : public ChooseFunction
{
public:
  virtual std::string compute(VariablePtr choice) const = 0;
};
typedef ReferenceCountedObjectPtr<ActionDescriptionFunction> ActionDescriptionFunctionPtr;

}; /* namespace cralgo */

#endif // !CRALGO_VALUE_FUNCTION_H_
