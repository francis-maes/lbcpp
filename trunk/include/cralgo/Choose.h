/*-----------------------------------------.---------------------------------.
| Filename: Choose.h                       | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CHOOSE_H_
# define CRALGO_CHOOSE_H_

# include "ChooseFunction.h"
# include "ChooseFunction.h"
# include "Variable.h"

namespace cralgo
{

class Choose : public Object
{
public:  
  ChoosePtr getReferenceCountedPointer() const
    {return ChoosePtr(const_cast<Choose* >(this));}

  /*
  ** Choices
  */
  virtual std::string getChoiceType() const = 0;
  virtual size_t getNumChoices() const = 0;
  virtual VariableIteratorPtr newIterator() const = 0;
  virtual VariablePtr sampleRandomChoice() const = 0;
  virtual VariablePtr sampleBestChoice(ActionValueFunctionPtr valueFunction) const = 0;

  /*
  ** CR-algorithm related
  */
  CRAlgorithmPtr getCRAlgorithm() const
    {return crAlgorithm;}
  
  void setCRAlgorithm(CRAlgorithmPtr crAlgorithm) 
    {this->crAlgorithm = crAlgorithm;}
  
  std::string computeStateDescription() const;
  std::string computeActionDescription(VariablePtr choice) const;
  
  double computeStateValue() const;
  double computeActionValue(VariablePtr choice) const;
  virtual void computeActionValues(std::vector<double>& res) const = 0;

  FeatureGeneratorPtr computeStateFeatures() const;
  FeatureGeneratorPtr computeActionFeatures(VariablePtr choice) const;
  virtual void computeActionFeatures(std::vector<FeatureGeneratorPtr>& res, bool transformIntoSparseVectors) const = 0;
    
  /*
  ** Composite functions
  */
  virtual StateDescriptionFunctionPtr   getStateDescriptionFunction() const = 0;
  virtual ActionDescriptionFunctionPtr  getActionDescriptionFunction() const = 0;
  virtual StateValueFunctionPtr         getStateValueFunction() const = 0;
  virtual ActionValueFunctionPtr        getActionValueFunction() const = 0;
  virtual StateFeaturesFunctionPtr      getStateFeaturesFunction() const = 0;
  virtual ActionFeaturesFunctionPtr     getActionFeaturesFunction() const = 0;

  /*
  ** Detailed functions
  */
  virtual size_t getNumStateDescriptions() const = 0;
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const = 0;
  virtual size_t getNumActionDescriptions() const = 0;
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const = 0;

  virtual size_t getNumStateValues() const = 0;
  virtual StateValueFunctionPtr getStateValue(size_t index) const = 0;
  virtual size_t getNumActionValues() const = 0;
  virtual ActionValueFunctionPtr getActionValue(size_t index) const = 0;

  virtual size_t getNumStateFeatures() const = 0;
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const = 0;
  virtual size_t getNumActionFeatures() const = 0;
  virtual ActionFeaturesFunctionPtr getActionFeatures(size_t index) const = 0;
  
  /*
  ** Object
  */
  virtual std::string Choose::toString() const;

protected:
  Choose(CRAlgorithmPtr crAlgorithm = CRAlgorithmPtr())
    : crAlgorithm(crAlgorithm) {}
    
  CRAlgorithmPtr crAlgorithm;
};

typedef ReferenceCountedObjectPtr<Choose> ChoosePtr;

}; /* namespace cralgo */

#endif // !CRALGO_CHOOSE_H_
