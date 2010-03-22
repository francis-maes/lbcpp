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
| Filename: Choose.h                       | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Choose.h
**@author Francis MAES
**@date   Fri Jun 12 17:01:02 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_CHOOSE_H_
# define LBCPP_CHOOSE_H_

# include "ChooseFunction.h"
# include "ChooseFunction.h"
# include "Variable.h"

namespace lbcpp
{

/*!
** @class Choose
** @brief #FIXME
**
*/
class Choose : public Object
{
public:
  /**
  ** Returns a reference on the current Choose instance.
  **
  ** @return a reference on the current Choose instance.
  */
  ChoosePtr getReferenceCountedPointer() const
    {return ChoosePtr(const_cast<Choose* >(this));}

  /*
  ** Choices
  */
  /**
  ** Returns the choice type.
  **
  ** @return the choice type.
  */
  virtual std::string getChoiceType() const = 0;

  /**
  ** Returns the number of available choices.
  **
  ** @return the number of available choices.
  */
  virtual size_t getNumChoices() const = 0;

  /**
  ** Returns a new iterartor on the choice list.
  **
  ** @return a new iterartor on the choice list.
  */
  virtual VariableIteratorPtr newIterator() const = 0;

  /**
  ** Returns a random choice.
  **
  ** @return a random choice.
  */
  virtual VariablePtr sampleRandomChoice() const = 0;

  /**
  ** Samples the best choice according to the @a valueFunction.
  **
  ** @param valueFunction : function that measures choices.
  **
  ** @return the best choice according to the @a valueFunction.
  */
  virtual VariablePtr sampleBestChoice(ActionValueFunctionPtr valueFunction) const = 0;

  /**
  ** Samples a choice given a discrete probability distribution.
  **
  ** @param probabilities : discrete probability distribution.
  ** @param probabilitiesSum : sum of the @a probabilites.
  **
  ** @return a choice given a discrete probability distribution.
  ** @see Random::sampleWithProbabilities
  */
  virtual VariablePtr sampleChoiceWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0) const = 0;

  /*
  ** CR-algorithm related
  */
  /**
  ** Returns the CRAlgorithm related.
  **
  ** @return a pointer on the CRAlgorithm related.
  */
  CRAlgorithmPtr getCRAlgorithm() const
    {return crAlgorithm;}

  /**
  ** Changes the CRAlgorithm related.
  **
  ** @param crAlgorithm : another CRAlgorithm.
  */
  void setCRAlgorithm(CRAlgorithmPtr crAlgorithm)
    {this->crAlgorithm = crAlgorithm;}

  /**
  ** Syntaxic sugar for "getStateDescriptionFunction()->compute()".
  **
  ** @return getStateDescriptionFunction()->compute() result.
  ** @see StateDescriptionFunction::compute. 
  */
  std::string computeStateDescription() const;

  /**
  ** Syntaxic sugar for "getActionDescriptionFunction()->compute(@a choice)".
  **
  ** @param choice : target choice.
  **
  ** @return getActionDescriptionFunction()->compute(@a choice) result.
  ** @see ActionDescriptionFunction::compute 
  */
  std::string computeActionDescription(VariablePtr choice) const;

  /**
  ** Syntaxic sugar for "getStateValueFunction()->compute()".
  **
  ** @return getStateValueFunction()->compute() result.
  ** @see StateValueFunction::compute
  */
  double computeStateValue() const;

  /**
  ** Syntaxic sugar for "getActionValueFunction()->compute(@a choice)".
  **
  ** @param choice : a choice.
  **
  ** @return getActionValueFunction()->compute(@a choice) result.
  ** @see ActionValueFunction::compute 
  */
  double computeActionValue(VariablePtr choice) const;

  /**
  ** #FIXME
  **
  ** @param res
  ** @param actionValues
  */
  virtual void computeActionValues(std::vector<double>& res, ActionValueFunctionPtr actionValues = ActionValueFunctionPtr()) const = 0;

  /**
  ** Syntaxic sugar for "getStateFeaturesFunction()->compute()".
  **
  ** @return getStateFeaturesFunction()->compute() result.
  ** @see StateFeaturesFunction::compute 
  */
  FeatureGeneratorPtr computeStateFeatures() const;

  /**
  ** Syntaxic sugar for "getActionFeaturesFunction()->compute(@a choice)".
  **
  ** @param choice : a choice.
  **
  ** @return getActionFeaturesFunction()->compute(@a choice) result.
  ** @see ActionFeaturesFunction:compute 
  ** @see FeatureGenerator 
  */
  FeatureGeneratorPtr computeActionFeatures(VariablePtr choice) const;

  /**
  ** #FIXME
  **
  ** @param transformIntoSparseVectors : 
  **
  ** @return
  ** @see FeatureGenerator 
  */
  virtual FeatureGeneratorPtr computeActionsFeatures(bool transformIntoSparseVectors) const = 0;

  /*
  ** Composite functions
  */
  /**
  ** Returns the StateDescriptionFunction of the current state.
  **
  ** @return the StateDescriptionFunction of the current state.
  ** @see StateDescriptionFunction 
  */
  virtual StateDescriptionFunctionPtr   getStateDescriptionFunction() const = 0;

  /**
  ** Returns the ActionDescriptionFunction of the current state.
  **
  ** @return the ActionDescriptionFunction of the current state.
  ** @see ActionDescriptionFunction 
  */
  virtual ActionDescriptionFunctionPtr  getActionDescriptionFunction() const = 0;

  /**
  ** Returns the StateValueFunction of the current state.
  **
  ** @return the StateValueFunction of the current state.
  ** @see StateValueFunction 
  */
  virtual StateValueFunctionPtr         getStateValueFunction() const = 0;

  /**
  ** Returns the ActionValueFunction of the current state.
  **
  ** @return the ActionValueFunction of the current state.
  ** @see ActionValueFunction 
  */
  virtual ActionValueFunctionPtr        getActionValueFunction() const = 0;

  /**
  ** Returns the StateFeatureFunction of the current state.
  **
  ** @return the StateFeatureFunction of the current state.
  ** @see StatefeaturesFunction 
  */
  virtual StateFeaturesFunctionPtr      getStateFeaturesFunction() const = 0;

  /**
  ** Returns the ActionFeaturesFunction of the current state.
  **
  ** @return the ActionFeaturesFunction of the current state.
  ** @see ActionFeaturesFunction
  */
  virtual ActionFeaturesFunctionPtr     getActionFeaturesFunction() const = 0;

  /*
  ** Detailed functions
  */
	
  /**
  ** Returns the number of StateDescription.
  **
  ** @return the number of StateDescription.
  */
  virtual size_t getNumStateDescriptions() const = 0;

  /**
  ** Returns the StateDescriptionFunction number @a index of the current state.
  **
  ** @param index : state number. 
  **
  ** @return the StateDescriptionFunction number @a index of the current state.
  ** @see StateDescriptionFunction 
  */
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const = 0;

  /**
  ** Returns the number of ActionDescription.
  **
  ** @return the number of ActionDescription.
  */
  virtual size_t getNumActionDescriptions() const = 0;

  /**
  ** Returns the ActionDescriptionFunction number @a index of the current state.
  **
  ** @param index : action number.
  **
  ** @return the ActionDescriptionFunction number @a index of the current state.
  ** @see ActionDescriptionFunction 
  */
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const = 0;

  /**
  ** Returns the number of StateValues.
  **
  ** @return the number of StateValues.
  */
  virtual size_t getNumStateValues() const = 0;

  /**
  ** Returns the StateValueFunction of state number @a index.
  **
  ** @param index : state number.
  **
  ** @return the StateValueFunction of state number @a index.
  ** @see StateValueFunction 
  */
  virtual StateValueFunctionPtr getStateValue(size_t index) const = 0;

  /**
  ** Returns the number of ActionValues.
  **
  ** @return the number of ActionValues.
  */
  virtual size_t getNumActionValues() const = 0;

  /**
  ** Returns the ActionValueFunction of the action number @a index.
  **
  ** @param index : action number.
  **
  ** @return the ActionValueFunction of the action number @a index.
  ** @see ActionValueFunction 
  */
  virtual ActionValueFunctionPtr getActionValue(size_t index) const = 0;

  /**
  ** Returns the number of StateFeatures.
  **
  ** @return the number of StateFeatures.
  ** @see StateFeatures 
  */
  virtual size_t getNumStateFeatures() const = 0;

  /**
  ** Returns the StateFeatures at the index @a index.
  **
  ** @param index : StateFeatures index.
  **
  ** @return the StateFeatures at the index @a index.
  ** @see StateFeaturesFunction 
  */
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const = 0;

  /**
  ** Returns the number of ActionFeatures.
  **
  ** @return the number of ActionFeatures.
  ** @see ActionFeatures 
  */
  virtual size_t getNumActionFeatures() const = 0;

  /**
  ** Returns the ActionFeatures at the index @a index.
  **
  ** @param index : ActionFeatures index.
  **
  ** @return the ActionFeatures at the index @a index.
  ** @see ActionFeaturesFunction 
  */
  virtual ActionFeaturesFunctionPtr getActionFeatures(size_t index) const = 0;

  /*
  ** Object
  */
  /**
  ** Converts choices to a String.
  **
  ** @return the conversion of choices to a string.
  */
  virtual std::string toString() const;

protected:
  Choose(CRAlgorithmPtr crAlgorithm = CRAlgorithmPtr())
    : crAlgorithm(crAlgorithm) {}

  CRAlgorithmPtr crAlgorithm;
};


typedef ReferenceCountedObjectPtr<Choose> ChoosePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CHOOSE_H_
