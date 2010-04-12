/*-----------------------------------------.---------------------------------.
| Filename: StackedClassificationVaria....h| Stacked Classification          |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2010 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_SET_MODEL_STACKED_CLASSIFICATION_H_
# define LBCPP_VARIABLE_SET_MODEL_STACKED_CLASSIFICATION_H_

# include "../VariableSetModel.h"

namespace lbcpp
{

class StackedClassificationVariableSetModel : public VariableSetModel
{
public:
  StackedClassificationVariableSetModel(VariableSetModelPtr crossValidatedModel, VariableSetModelPtr decisionModel)
    : crossValidatedModel(crossValidatedModel), decisionModel(decisionModel) {}
    
  // todo
    
private:
  VariableSetModelPtr crossValidatedModel;
  VariableSetModelPtr decisionModel;
};


}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_STACKED_CLASSIFICATION_H_

