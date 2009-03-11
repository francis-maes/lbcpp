/*-----------------------------------------.---------------------------------.
| Filename: Choose.h                       | Choose                          |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CHOOSE_H_
# define CRALGO_CHOOSE_H_

# include "Object.h"
# include "StateFunction.h"

namespace cralgo
{

class ActionIterator : public ReferenceCountedObject
{
public:
  virtual ~ActionIterator() {}
  
  virtual bool exists() const = 0;
  virtual const void* get() const = 0;
  virtual void next() = 0;
};

typedef ReferenceCountedObjectPtr<ActionIterator> ActionIteratorPtr;

class Choose : public Object
{
public:  
  /*
  ** Choices
  */
  virtual std::string getChoiceType() const = 0;
  virtual size_t getNumChoices() const = 0;
  virtual ActionIteratorPtr newIterator() const = 0;
  virtual void* cloneChoice(const void* choice) const = 0;
  virtual void deleteChoice(const void* choice) const = 0;
  virtual const void* sampleRandomChoice() const = 0;
  virtual const void* sampleBestChoice(ActionValueFunctionPtr valueFunction) const = 0;

  /*
  ** CR-algorithm related
  */
  CRAlgorithmPtr getCRAlgorithm() const
    {return crAlgorithm;}
  
  void setCRAlgorithm(CRAlgorithmPtr crAlgorithm) 
    {this->crAlgorithm = crAlgorithm;}
  
  std::string stateDescription() const
    {return getStateDescriptionFunction()->toString(crAlgorithm);}

  std::string actionDescription(const void* choice) const
    {return getActionDescriptionFunction()->toString(crAlgorithm, choice);}

  double stateValue() const
    {return getStateValueFunction()->compute(crAlgorithm);}

  double actionValue(const void* choice) const
    {return getActionValueFunction()->compute(crAlgorithm, choice);}
    
  FeatureGeneratorPtr stateFeatures() const
    {return getStateFeaturesFunction()->featureGenerator(crAlgorithm);}
    
  FeatureGeneratorPtr actionFeatures(const void* choice) const
    {return getActionFeaturesFunction()->featureGenerator(crAlgorithm, choice);}

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
  ** String descriptions
  */
  virtual size_t getNumStateDescriptions() const = 0;
  virtual StateDescriptionFunctionPtr getStateDescription(size_t index) const = 0;
  virtual size_t getNumActionDescriptions() const = 0;
  virtual ActionDescriptionFunctionPtr getActionDescription(size_t index) const = 0;

  /*
  ** Value functions
  */
  virtual size_t getNumStateValues() const = 0;
  virtual StateValueFunctionPtr getStateValue(size_t index) const = 0;
  virtual size_t getNumActionValues() const = 0;
  virtual ActionValueFunctionPtr getActionValue(size_t index) const = 0;

  /*
  ** Features
  */
  virtual size_t getNumStateFeatures() const = 0;
  virtual StateFeaturesFunctionPtr getStateFeatures(size_t index) const = 0;
  virtual size_t getNumActionFeatures() const = 0;
  virtual ActionFeaturesFunctionPtr getActionFeatures(size_t index) const = 0;
  
protected:
  Choose(CRAlgorithmPtr crAlgorithm = CRAlgorithmPtr())
    : crAlgorithm(crAlgorithm) {}
    
  CRAlgorithmPtr crAlgorithm;
};

typedef ReferenceCountedObjectPtr<Choose> ChoosePtr;

}; /* namespace cralgo */

#endif // !CRALGO_CHOOSE_H_
