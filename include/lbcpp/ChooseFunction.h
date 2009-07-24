/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
  /**
  ** Choice setter.
  **
  ** @param choose : a choice.
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
  /**
  **
  **
  **
  ** @return
  */
  virtual double compute() const = 0;
};

extern StateValueFunctionPtr chooseStateValues();

/**
**
**
** @param regressor
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RegressorPtr regressor);

/**
**
**
** @param regressor
**
** @return
*/
inline StateValueFunctionPtr predictedStateValues(GradientBasedRegressorPtr regressor)
  {return predictedStateValues(RegressorPtr(regressor));}

/**
**
**
** @param ranker
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RankerPtr ranker);

/**
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
  /**
  **
  **
  ** @param choice
  **
  ** @return
  */
  virtual double compute(VariablePtr choice) const = 0;
};

/**
**
**
**
** @return
*/
extern ActionValueFunctionPtr chooseActionValues();

extern ActionValueFunctionPtr stateValueBasedActionValues(StateValueFunctionPtr stateValues, double discount = 1.0);

/**
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(ClassifierPtr classifier);

/**
**
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedClassifierPtr classifier)
  {return predictedActionValues(ClassifierPtr(classifier));}

/**
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(GeneralizedClassifierPtr classifier);

/**
**
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedGeneralizedClassifierPtr classifier)
  {return predictedActionValues(GeneralizedClassifierPtr(classifier));}

/**
**
**
** @param ranker
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RankerPtr ranker);

/**
**
**
** @param ranker
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRankerPtr ranker)
  {return predictedActionValues(RankerPtr(ranker));}

/**
**
**
** @param regressor
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RegressorPtr regressor);

/**
**
**
** @param regressor
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRegressorPtr regressor)
  {return predictedActionValues(RegressorPtr(regressor));}

/**
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(ClassifierPtr classifier);

/**
**
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(GeneralizedClassifierPtr classifier);

/**
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
  /**
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
  /**
  **
  **
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /**
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
  /**
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
  /**
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
