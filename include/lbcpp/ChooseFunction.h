/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunction.h               | Functions that depend on a      |
| Author  : Francis Maes                   |   choose                        |
| Started : 12/03/2009 17:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CHOOSE_FUNCTION_H_
# define LBCPP_CHOOSE_FUNCTION_H_

# include "ObjectPredeclarations.h"
# include "GradientBasedLearningMachine.h"

namespace lbcpp
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
** State Values
*/
class StateValueFunction : public ChooseFunction
{
public:
  virtual double compute() const = 0;
};

extern StateValueFunctionPtr predictedStateValues(RegressorPtr regressor);
inline StateValueFunctionPtr predictedStateValues(GradientBasedRegressorPtr regressor)
  {return predictedStateValues(RegressorPtr(regressor));}
  
extern StateValueFunctionPtr predictedStateValues(RankerPtr ranker);
inline StateValueFunctionPtr predictedStateValues(GradientBasedRankerPtr ranker)
  {return predictedStateValues(RankerPtr(ranker));}

/*
** Action Values
*/
class ActionValueFunction : public ChooseFunction
{
public:
  virtual double compute(VariablePtr choice) const = 0;
};

extern ActionValueFunctionPtr chooseActionValues();

extern ActionValueFunctionPtr predictedActionValues(ClassifierPtr classifier);
inline ActionValueFunctionPtr predictedActionValues(GradientBasedClassifierPtr classifier)
  {return predictedActionValues(ClassifierPtr(classifier));}

extern ActionValueFunctionPtr predictedActionValues(GeneralizedClassifierPtr classifier);
inline ActionValueFunctionPtr predictedActionValues(GradientBasedGeneralizedClassifierPtr classifier)
  {return predictedActionValues(GeneralizedClassifierPtr(classifier));}

extern ActionValueFunctionPtr predictedActionValues(RankerPtr ranker);    
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRankerPtr ranker)
  {return predictedActionValues(RankerPtr(ranker));}

extern ActionValueFunctionPtr predictedActionValues(RegressorPtr regressor);
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRegressorPtr regressor)
  {return predictedActionValues(RegressorPtr(regressor));}

extern ActionValueFunctionPtr probabilitiesActionValues(ClassifierPtr classifier);
extern ActionValueFunctionPtr probabilitiesActionValues(GeneralizedClassifierPtr classifier);
inline ActionValueFunctionPtr probabilitiesActionValues(GradientBasedClassifierPtr classifier)
  {return probabilitiesActionValues(ClassifierPtr(classifier));}

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
  virtual FeatureDictionaryPtr getDictionary() const = 0;
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

}; /* namespace lbcpp */

#endif // !LBCPP_VALUE_FUNCTION_H_
