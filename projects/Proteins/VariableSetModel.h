/*-----------------------------------------.---------------------------------.
| Filename: VariableSetModel.h             | VariableSet Model               |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2010 17:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_SET_MODEL_H_
# define LBCPP_VARIABLE_SET_MODEL_H_

#include "GeneratedCode/Data/Generic/VariableSet.lh"

namespace lbcpp
{

class VariableSetModel : public LearningMachine
{
public:
  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const = 0;
  
    // evaluate the accuracy for the moment
  double evaluate(ObjectContainerPtr examples) const;
};

typedef ReferenceCountedObjectPtr<VariableSetModel> VariableSetModelPtr;

extern VariableSetModelPtr independantClassificationVariableSetModel(ClassifierPtr classifier);
extern VariableSetModelPtr iterativeClassificationVariableSetModel(ClassifierPtr initialClassifier, ClassifierPtr iterativeClassifier);
extern VariableSetModelPtr simulatedIterativeClassificationVariableSetModel(ClassifierPtr stochasticClassifier, StoppingCriterionPtr stoppingCriterion);

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_H_