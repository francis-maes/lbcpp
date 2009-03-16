/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunction.h               | Functions that depend on a      |
| Author  : Francis Maes                   |   choose                        |
| Started : 12/03/2009 17:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CHOOSE_FUNCTION_H_
# define CRALGO_CHOOSE_FUNCTION_H_

# include "ObjectPredeclarations.h"

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

/*
** Values
*/
class StateValueFunction : public ChooseFunction
{
public:
  static StateValueFunctionPtr createPredictions(RegressorPtr regressor);

  virtual double compute() const = 0;
};

class ActionValueFunction : public ChooseFunction
{
public:
  static ActionValueFunctionPtr createScores(ClassifierPtr classifier);
  static ActionValueFunctionPtr createScores(GeneralizedClassifierPtr classifier);
  static ActionValueFunctionPtr createProbabilities(ClassifierPtr classifier);
  static ActionValueFunctionPtr createPredictions(RegressorPtr regressor);

  virtual double compute(VariablePtr choice) const = 0;
};

/*
** Features
*/
class StateFeaturesFunction : public ChooseFunction
{
public:
  virtual FeatureGeneratorPtr compute() const = 0;
};

class ActionFeaturesFunction : public ChooseFunction
{
public:
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const = 0;
};

/*
** String Descriptions
*/
class StateDescriptionFunction : public ChooseFunction
{
public:
  virtual std::string compute() const = 0;
};

class ActionDescriptionFunction : public ChooseFunction
{
public:
  virtual std::string compute(VariablePtr choice) const = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_VALUE_FUNCTION_H_
