/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunction.h               | Functions that depend on a      |
| Author  : Francis Maes                   |   choose                        |
| Started : 12/03/2009 17:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ChooseFunction.h
**@author Francis MAES
**@date   Fri Jun 12 17:04:39 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_CHOOSE_FUNCTION_H_
# define LBCPP_CHOOSE_FUNCTION_H_

# include "ObjectPredeclarations.h"
# include "GradientBasedLearningMachine.h"

namespace lbcpp
{

/*!
** @class ChooseFunction
** @brief ChooseFunction base class.
**
*/
class ChooseFunction : public Object
{
public:
  /*!
  **
  **
  ** @param choose
  */
  virtual void setChoose(ChoosePtr choose) = 0;
};

/*
** State Values
*/
/*!
** @class StateValueFunction
** @brief State values
**
*/
class StateValueFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual double compute() const = 0;
};

extern StateValueFunctionPtr chooseStateValues();

/*!
**
**
** @param regressor
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RegressorPtr regressor);

/*!
**
**
** @param regressor
**
** @return
*/
inline StateValueFunctionPtr predictedStateValues(GradientBasedRegressorPtr regressor)
  {return predictedStateValues(RegressorPtr(regressor));}

/*!
**
**
** @param ranker
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RankerPtr ranker);

/*!
**
**
** @param ranker
**
** @return
*/
inline StateValueFunctionPtr predictedStateValues(GradientBasedRankerPtr ranker)
  {return predictedStateValues(RankerPtr(ranker));}


extern StateValueFunctionPtr simulationStateValues(PolicyPtr policy, double discount = 1, size_t horizon = 0);


/*!
** @class ActionValueFunction
** @brief Action values
**
*/
class ActionValueFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  virtual double compute(VariablePtr choice) const = 0;
};

/*!
**
**
**
** @return
*/
extern ActionValueFunctionPtr chooseActionValues();

extern ActionValueFunctionPtr stateValueBasedActionValues(StateValueFunctionPtr stateValues, double discount = 1.0);

/*!
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(ClassifierPtr classifier);

/*!
**
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedClassifierPtr classifier)
  {return predictedActionValues(ClassifierPtr(classifier));}

/*!
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(GeneralizedClassifierPtr classifier);

/*!
**
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedGeneralizedClassifierPtr classifier)
  {return predictedActionValues(GeneralizedClassifierPtr(classifier));}

/*!
**
**
** @param ranker
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RankerPtr ranker);

/*!
**
**
** @param ranker
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRankerPtr ranker)
  {return predictedActionValues(RankerPtr(ranker));}

/*!
**
**
** @param regressor
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RegressorPtr regressor);

/*!
**
**
** @param regressor
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRegressorPtr regressor)
  {return predictedActionValues(RegressorPtr(regressor));}

/*!
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(ClassifierPtr classifier);

/*!
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(GeneralizedClassifierPtr classifier);

/*!
**
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr probabilitiesActionValues(GradientBasedClassifierPtr classifier)
  {return probabilitiesActionValues(ClassifierPtr(classifier));}

/*
** Features
*/
/*!
** @class StatFeaturesFunction
** @brief Features
**
*/
class StateFeaturesFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual FeatureGeneratorPtr compute() const = 0;
};

/*!
** @class ActionFeaturesFunction
** @brief #FIXME
**
*/
class ActionFeaturesFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const = 0;
};

/*
** String Descriptions
** deprecated
*/
/*!
** @class StateDescriptionFunction
** @brief String descriptions
**
*/
class StateDescriptionFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string compute() const = 0;
};

/*!
** @class ActionDescriptionFunction
** @brief #FIXME
**
*/
class ActionDescriptionFunction : public ChooseFunction
{
public:
  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  virtual std::string compute(VariablePtr choice) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_VALUE_FUNCTION_H_
