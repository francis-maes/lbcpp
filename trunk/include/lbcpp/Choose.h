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
  /*!
  **
  **
  **
  ** @return
  */
  ChoosePtr getReferenceCountedPointer() const
    {return ChoosePtr(const_cast<Choose* >(this));}

  /*
  ** Choices
  */
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string getChoiceType() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumChoices() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual VariableIteratorPtr newIterator() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual VariablePtr sampleRandomChoice() const = 0;

  /*!
  **
  **
  ** @param valueFunction
  **
  ** @return
  */
  virtual VariablePtr sampleBestChoice(ActionValueFunctionPtr valueFunction) const = 0;

  /*!
  **
  **
  ** @param probabilities
  ** @param probabilitiesSum
  **
  ** @return
  */
  virtual VariablePtr sampleChoiceWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0) const = 0;

  /*
  ** CR-algorithm related
  */
  /*!
  **
  **
  **
  ** @return
  */
  CRAlgorithmPtr getCRAlgorithm() const
    {return crAlgorithm;}

  /*!
  **
  **
  ** @param crAlgorithm
  */
  void setCRAlgorithm(CRAlgorithmPtr crAlgorithm)
    {this->crAlgorithm = crAlgorithm;}

  /*!
  **
  **
  **
  ** @return
  */
  std::string computeStateDescription() const;

  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  std::string computeActionDescription(VariablePtr choice) const;

  /*!
  **
  **
  **
  ** @return
  */
  double computeStateValue() const;

  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  double computeActionValue(VariablePtr choice) const;

  /*!
  **
  **
  ** @param res
  ** @param actionValues
  */
  virtual void computeActionValues(std::vector<double>& res, ActionValueFunctionPtr actionValues = ActionValueFunctionPtr()) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  FeatureGeneratorPtr computeStateFeatures() const;

  /*!
  **
  **
  ** @param choice
  **
  ** @return
  */
  FeatureGeneratorPtr computeActionFeatures(VariablePtr choice) const;

  /*!
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
  /*!
  **
  **
  **
  ** @return
  */
  virtual StateDescriptionFunctionPtr   getStateDescriptionFunction() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual ActionDescriptionFunctionPtr  getActionDescriptionFunction() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual StateValueFunctionPtr         getStateValueFunction() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual ActionValueFunctionPtr        getActionValueFunction() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual StateFeaturesFunctionPtr      getStateFeaturesFunction() const = 0;

  /*!
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

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionDescriptions() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumStateValues() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateValueFunctionPtr getStateValue(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionValues() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ActionValueFunctionPtr getActionValue(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumStateFeatures() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumActionFeatures() const = 0;

  /*!
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
  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const;

protected:
  /*!
  **
  **
  ** @param crAlgorithm
  **
  ** @return
  */
  Choose(CRAlgorithmPtr crAlgorithm = CRAlgorithmPtr())
    : crAlgorithm(crAlgorithm) {}

  CRAlgorithmPtr crAlgorithm;   /*!< */
};


typedef ReferenceCountedObjectPtr<Choose> ChoosePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CHOOSE_H_
