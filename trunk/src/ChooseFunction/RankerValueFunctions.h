/*-----------------------------------------.---------------------------------.
| Filename: RankerValueFunction.h          | Ranker-based value functions    |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 19:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_RANKER_H_
# define LBCPP_CORE_VALUE_FUNCTION_RANKER_H_

# include <lbcpp/ChooseFunction.h>
# include <lbcpp/LearningMachine.h>

namespace lbcpp
{

class RankerStateValueFunction : public StateValueFunction
{
public:
  RankerStateValueFunction(RankerPtr ranker = RankerPtr())
    : ranker(ranker) {}
  
  virtual String toString() const
    {return "predictedStateValues(" + ranker->toString() + ")";}
    
  virtual void setChoose(ChoosePtr choose)
    {stateFeatures = choose->computeStateFeatures();}

  virtual double compute() const
    {return ranker->predictScore(stateFeatures);}

  virtual bool load(std::istream& istr)
    {return read(istr, ranker);}

  virtual void save(std::ostream& ostr)
    {write(ostr, ranker);}

protected:
  RankerPtr ranker;
  FeatureGeneratorPtr stateFeatures;
};

class RankerActionValueFunction : public ActionValueFunction
{
public:
  RankerActionValueFunction(RankerPtr ranker = RankerPtr())
    : ranker(ranker) {}
  
  virtual String toString() const
    {return "predictedActionValues(" + ranker->toString() + ")";}
    
  virtual void setChoose(ChoosePtr choose)
    {actionFeatures = choose->getActionFeaturesFunction();}

  virtual double compute(VariablePtr variable) const
    {return ranker->predictScore(actionFeatures->compute(variable));}

  virtual bool load(std::istream& istr)
    {return read(istr, ranker);}

  virtual void save(std::ostream& ostr)
    {write(ostr, ranker);}

protected:
  RankerPtr ranker;
  ActionFeaturesFunctionPtr actionFeatures;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_RANKER_H_
