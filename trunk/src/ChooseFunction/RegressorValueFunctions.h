/*-----------------------------------------.---------------------------------.
| Filename: RegressorValueFunction.h       | Regressor-based value functions |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_REGRESSOR_H_
# define LBCPP_CORE_VALUE_FUNCTION_REGRESSOR_H_

# include <lbcpp/CRAlgorithm/ChooseFunction.h>
# include <lbcpp/LearningMachine.h>

namespace lbcpp
{

class RegressorStateValueFunction : public StateValueFunction
{
public:
  RegressorStateValueFunction(RegressorPtr regressor = RegressorPtr())
    : regressor(regressor) {}
    
  virtual String toString() const
    {return "predictedStateValues(" + regressor->toString() + ")";}
  
  virtual void setChoose(ChoosePtr choose)
    {stateFeatures = choose->computeStateFeatures();}

  virtual double compute() const
    {return regressor->predict(stateFeatures);}

  virtual bool load(InputStream& istr)
    {return read(istr, regressor);}

  virtual void save(OutputStream& ostr)
    {write(ostr, regressor);}

private:
  RegressorPtr regressor;
  FeatureGeneratorPtr stateFeatures;
};

class RegressorActionValueFunction : public ActionValueFunction
{
public:  
  RegressorActionValueFunction(RegressorPtr regressor = RegressorPtr())
    : regressor(regressor) {}
  
  virtual String toString() const
    {return "predictedActionValues(" + regressor->toString() + ")";}

  virtual void setChoose(ChoosePtr choose)
    {actionFeatures = choose->getActionFeaturesFunction();}

  virtual double compute(VariablePtr choice) const
    {return regressor->predict(actionFeatures->compute(choice));}

  virtual bool load(InputStream& istr)
    {return read(istr, regressor);}

  virtual void save(OutputStream& ostr)
    {write(ostr, regressor);}

private:
  RegressorPtr regressor;
  ActionFeaturesFunctionPtr actionFeatures;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_REGRESSOR_H_
