/*-----------------------------------------.---------------------------------.
| Filename: ChooseFunction.cpp             | Choose Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 22/06/2009 19:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/CRAlgorithm.h>
#include "ChooseFunction/ChooseValueFunctions.h"
#include "ChooseFunction/ClassifierValueFunctions.h"
#include "ChooseFunction/RankerValueFunctions.h"
#include "ChooseFunction/RegressorValueFunctions.h"
#include "ChooseFunction/SimulationStateValueFunction.h"
#include "ChooseFunction/StateValueBasedActionValueFunction.h"
using namespace lbcpp;

/*
** StateValueFunction
*/
StateValueFunctionPtr lbcpp::chooseStateValues()
  {return new ChooseStateValueFunction();}

StateValueFunctionPtr lbcpp::predictedStateValues(RegressorPtr regressor)
  {return new RegressorStateValueFunction(regressor);}

StateValueFunctionPtr lbcpp::predictedStateValues(RankerPtr ranker)
  {return new RankerStateValueFunction(ranker);}
  
StateValueFunctionPtr lbcpp::simulationStateValues(PolicyPtr policy, double discount, size_t horizon)
  {return new SimulationStateValueFunction(policy, discount, horizon);}

/*
** ActionValueFunction
*/
ActionValueFunctionPtr lbcpp::chooseActionValues()
  {return new ChooseActionValueFunction();}

ActionValueFunctionPtr lbcpp::predictedActionValues(ClassifierPtr classifier)
  {return new ClassifierScoresActionValue(classifier);}

ActionValueFunctionPtr lbcpp::predictedActionValues(GeneralizedClassifierPtr classifier)
  {return new GeneralizedClassifierScoresActionValue(classifier);}

ActionValueFunctionPtr lbcpp::predictedActionValues(RankerPtr ranker)
  {return new RankerActionValueFunction(ranker);}

ActionValueFunctionPtr lbcpp::predictedActionValues(RegressorPtr regressor)
  {return new RegressorActionValueFunction(regressor);}

ActionValueFunctionPtr lbcpp::probabilitiesActionValues(ClassifierPtr classifier)
  {return new ClassifierProbabilitiesActionValue(classifier);}

ActionValueFunctionPtr lbcpp::probabilitiesActionValues(GeneralizedClassifierPtr classifier)
  {return new GeneralizedClassifierProbabilitiesActionValue(classifier);}

ActionValueFunctionPtr lbcpp::stateValueBasedActionValues(StateValueFunctionPtr stateValues, double discount)
  {return new StateValueBasedActionValueFunction(stateValues, discount);}

/*
** Serializable classes declaration
*/
void declareChooseFunctions()
{
  LBCPP_DECLARE_CLASS(ChooseStateValueFunction);
  LBCPP_DECLARE_CLASS(RegressorStateValueFunction);
  LBCPP_DECLARE_CLASS(RankerStateValueFunction);
  LBCPP_DECLARE_CLASS(SimulationStateValueFunction);
  
  LBCPP_DECLARE_CLASS(ChooseActionValueFunction);
  LBCPP_DECLARE_CLASS(StateValueBasedActionValueFunction);
  LBCPP_DECLARE_CLASS(ClassifierScoresActionValue);
  LBCPP_DECLARE_CLASS(GeneralizedClassifierScoresActionValue);
  LBCPP_DECLARE_CLASS(RankerActionValueFunction);
  LBCPP_DECLARE_CLASS(RegressorActionValueFunction);
  LBCPP_DECLARE_CLASS(ClassifierProbabilitiesActionValue);
  LBCPP_DECLARE_CLASS(GeneralizedClassifierProbabilitiesActionValue);
}
