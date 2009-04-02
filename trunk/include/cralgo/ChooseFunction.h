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
  static StateValueFunctionPtr createPredictions(GradientBasedRegressorPtr regressor)
    {return createPredictions(RegressorPtr(regressor));}
    
  static StateValueFunctionPtr createPredictions(RankerPtr ranker);
  static StateValueFunctionPtr createPredictions(GradientBasedRankerPtr ranker)
    {return createPredictions(RankerPtr(ranker));}

  virtual double compute() const = 0;
};

class ActionValueFunction : public ChooseFunction
{
public:
  static ActionValueFunctionPtr createProbabilities(ClassifierPtr classifier);
  static ActionValueFunctionPtr createProbabilities(GeneralizedClassifierPtr classifier);
  static ActionValueFunctionPtr createScores(ClassifierPtr classifier);
  static ActionValueFunctionPtr createScores(GeneralizedClassifierPtr classifier);
  static ActionValueFunctionPtr createPredictions(RankerPtr ranker);    
  static ActionValueFunctionPtr createPredictions(RegressorPtr regressor);

  static ActionValueFunctionPtr createProbabilities(GradientBasedClassifierPtr classifier)
    {return createProbabilities(ClassifierPtr(classifier));}
  static ActionValueFunctionPtr createScores(GradientBasedClassifierPtr classifier)
    {return createScores(ClassifierPtr(classifier));}
  static ActionValueFunctionPtr createScores(GradientBasedGeneralizedClassifierPtr classifier)
    {return createScores(GeneralizedClassifierPtr(classifier));}
  static ActionValueFunctionPtr createPredictions(GradientBasedRankerPtr ranker)
    {return createPredictions(RankerPtr(ranker));}
  static ActionValueFunctionPtr createPredictions(GradientBasedRegressorPtr regressor)
    {return createPredictions(RegressorPtr(regressor));}

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
