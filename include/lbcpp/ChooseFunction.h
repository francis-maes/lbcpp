/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
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
**@brief  Functions that depend on a choose.
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
  ** Choose setter.
  **
  ** @param choose : a choose.
  */
  virtual void setChoose(ChoosePtr choose) = 0;
};

/*
** State Values
*/
/*!
** @class StateValueFunction
** @brief State quality measure.
**
**  
*/
class StateValueFunction : public ChooseFunction
{
public:
  /**
  ** Returns a float (double) value corresponding on the quality of the current state.
  **
  ** @return a float (double) value corresponding on the quality of the current state.
  */
  virtual double compute() const = 0;
};

/**
** #FIXME
**
** @return
*/	
extern StateValueFunctionPtr chooseStateValues();

/**
** #FIXME
**
** @param regressor
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RegressorPtr regressor);

/**
** #FIXME
**
** @param regressor
**
** @return
*/
inline StateValueFunctionPtr predictedStateValues(GradientBasedRegressorPtr regressor)
  {return predictedStateValues(RegressorPtr(regressor));}

/**
** #FIXME
**
** @param ranker
**
** @return
*/
extern StateValueFunctionPtr predictedStateValues(RankerPtr ranker);

/**
** #FIXME
**
** @param ranker
**
** @return
*/
inline StateValueFunctionPtr predictedStateValues(GradientBasedRankerPtr ranker)
  {return predictedStateValues(RankerPtr(ranker));}


/**
** #FIXME
**
** @param policy :
** @param discount :
** @param horzon :
**
** @return
*/
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
  ** Returns a float (double) value corresponding on the @a choice quality.
  **
  ** @param choice : choice to estimate.
  ** @return a float (double) value corresponding on the quality of the current state.
  */
  virtual double compute(VariablePtr choice) const = 0;
};

/**
** #FIXME
**
**
** @return
*/
extern ActionValueFunctionPtr chooseActionValues();

extern ActionValueFunctionPtr stateValueBasedActionValues(StateValueFunctionPtr stateValues, double discount = 1.0);

/**
** #FIXME
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(ClassifierPtr classifier);

/**
** #FIXME
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedClassifierPtr classifier)
  {return predictedActionValues(ClassifierPtr(classifier));}

/**
** #FIXME
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(GeneralizedClassifierPtr classifier);

/**
** #FIXME
**
** @param classifier
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedGeneralizedClassifierPtr classifier)
  {return predictedActionValues(GeneralizedClassifierPtr(classifier));}

/**
** #FIXME
**
** @param ranker
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RankerPtr ranker);

/**
** #FIXME
**
** @param ranker
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRankerPtr ranker)
  {return predictedActionValues(RankerPtr(ranker));}

/**
** #FIXME
**
** @param regressor
**
** @return
*/
extern ActionValueFunctionPtr predictedActionValues(RegressorPtr regressor);

/**
** #FIXME
**
** @param regressor
**
** @return
*/
inline ActionValueFunctionPtr predictedActionValues(GradientBasedRegressorPtr regressor)
  {return predictedActionValues(RegressorPtr(regressor));}

/** 
** #FIXME
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(ClassifierPtr classifier);

/**
** #FIXME
**
** @param classifier
**
** @return
*/
extern ActionValueFunctionPtr probabilitiesActionValues(GeneralizedClassifierPtr classifier);

/**
** #FIXME
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
** @brief State conversion class to FeatureGenerator.
**
*/
class StateFeaturesFunction : public ChooseFunction
{
public:
  /**
  ** Computes a FeatureGenerator describeing the current state.
  **
  ** @return a FeatureGenerator describeing the current state.
  ** @see FeatureGenerator 
  */
  virtual FeatureGeneratorPtr compute() const = 0;
};

/*!
** @class ActionFeaturesFunction
** @brief Action conversion class to FeatureGenerator.
**
*/
class ActionFeaturesFunction : public ChooseFunction
{
public:
  /**
  ** Returns the FeatureDictionary of the current Actions.
  **
  ** @return the FeatureDictionary of the current Actions.
  ** @see FeatureDictionary 
  */
  virtual FeatureDictionaryPtr getDictionary() const = 0;

  /**
  ** Computes a FeatureGenerator describeing the choice
  **
  ** @param choice
  **
  ** @return
  ** @see FeatureGenerator 
  */
  virtual FeatureGeneratorPtr compute(VariablePtr choice) const = 0;
};

/*
** String Descriptions
** deprecated
*/
/*!
** @class StateDescriptionFunction
** @brief String description of the States.
**
*/
class StateDescriptionFunction : public ChooseFunction
{
public:
  /**
  ** Computes the String description of the current state.
  **
  ** @return the String description of the current state.
  */
  virtual std::string compute() const = 0;
};

/*!
** @class ActionDescriptionFunction
** @brief String description of the Actions.
**
*/
class ActionDescriptionFunction : public ChooseFunction
{
public:
  /**
  ** Computes the String description of choices at the @a choice node.
  **
  ** @param choice : choice node.
  **
  ** @return the String description of choices at the @a choice node.
  */
  virtual std::string compute(VariablePtr choice) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_VALUE_FUNCTION_H_
