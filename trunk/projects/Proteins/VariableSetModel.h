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
  
  virtual void trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr());
    
protected:
  virtual void trainBatchIteration(ObjectContainerPtr examples) = 0;
};

typedef ReferenceCountedObjectPtr<VariableSetModel> VariableSetModelPtr;

extern VariableSetModelPtr independantClassificationVariableSetModel(ClassifierPtr classifier);
extern VariableSetModelPtr optimisticClassificationVariableSetModel(ClassifierPtr classifier);

extern VariableSetModelPtr iterativeClassificationVariableSetModel(ClassifierPtr initialClassifier, ClassifierPtr iterativeClassifier);
extern VariableSetModelPtr simulatedIterativeClassificationVariableSetModel(ClassifierPtr classifier,
                                                                            size_t maxInferencePasses = 5,
                                                                            bool randomOrderInference = true,
                                                                            bool deterministicLearning = false);

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_H_
