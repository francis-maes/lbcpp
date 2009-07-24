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
  ** Computes the CRAlgorithm state description.
  **
  ** @return the CRAlgorithm state description.
  */
  std::string computeStateDescription() const;

  /**
  ** Computes the description of the action related to @a choice.
  **
  ** @param choice : target choice.
  **
  ** @return the description of the action related to @a choice.
  */
  std::string computeActionDescription(VariablePtr choice) const;

  /**
  **
  **
  **
  ** @return
  */
  double computeStateValue() const;

  /**
  **
  **
  ** @param choice
  **
  ** @return
  */
  double computeActionValue(VariablePtr choice) const;

  /**
  **
  **
  ** @param res
  ** @param actionValues
  */
  virtual void computeActionValues(std::vector<double>& res, ActionValueFunctionPtr actionValues = ActionValueFunctionPtr()) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  FeatureGeneratorPtr computeStateFeatures() const;

  /**
  **
  **
  ** @param choice
  **
  ** @return
  */
  FeatureGeneratorPtr computeActionFeatures(VariablePtr choice) const;

  /**
  **
  **
  ** @param transformIntoSparseVectors
  **
  ** @return
  */
  virtual FeatureGeneratorPtr computeActionsFeatures(bool transformIntoSparseVectors) const = 0;

  /*
  ** Composite functions
  */
  /**
  **
  **
  **
  ** @return
  */
  virtual StateDescriptionFunctionPtr   getStateDescriptionFunction() const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual ActionDescriptionFunctionPtr  getActionDescriptionFunction() const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual StateValueFunctionPtr         getStateValueFunction() const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual ActionValueFunctionPtr        getActionValueFunction() const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual StateFeaturesFunctionPtr      getStateFeaturesFunction() const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual ActionFeaturesFunctionPtr     getActionFeaturesFunction() const = 0;

  /*
  ** Detailed functions
  */
  virtual size_t getNumStateDescriptions() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionDescriptions() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumStateValues() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateValueFunctionPtr getStateValue(size_t index) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionValues() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ActionValueFunctionPtr getActionValue(size_t index) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumStateFeatures() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const = 0;

  /**
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionFeatures() const = 0;

  /**
  **
  **
  ** @param index
  **
  ** @return
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
