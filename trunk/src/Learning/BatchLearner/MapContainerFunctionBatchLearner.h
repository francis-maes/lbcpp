/*-----------------------------------------.---------------------------------.
| Filename: MapContainerFunctionBatchLea..h| A Learner for the mapContainer  |
| Author  : Francis Maes                   |   function                      |
| Started : 15/02/2011 19:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_MAP_CONTAINER_FUNCTION_H_
# define LBCPP_LEARNING_BATCH_LEARNER_MAP_CONTAINER_FUNCTION_H_

# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

extern ClassPtr mapContainerFunctionClass;

class MapContainerFunctionBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return mapContainerFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    FunctionPtr subFunction = function->getVariable(0).getObjectAndCast<Function>();
    jassert(subFunction);

    size_t size = 0;
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      ContainerPtr container = trainingData[i]->getVariable(0).getObjectAndCast<Container>();
      if (container)
        size += container->getNumElements();
    }

    ObjectVectorPtr subTrainingData = makeSubExamples(subFunction, trainingData);
    ObjectVectorPtr subValidationData = makeSubExamples(subFunction, validationData);
    return subFunction->train(context, subTrainingData, subValidationData);
  }

protected:
  ObjectVectorPtr makeSubExamples(FunctionPtr subFunction, const std::vector<ObjectPtr>& examples) const
  {
    if (!examples.size())
      return ObjectVectorPtr();

    ClassPtr inputsClass = subFunction->getInputsClass();
    ObjectVectorPtr res = new ObjectVector(inputsClass, computeNumSubExamples(examples));

    std::vector<ObjectPtr>& subExamples = res->getObjects();
    size_t index = 0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      const ObjectPtr& example = examples[i];
      
      ContainerPtr container = example->getVariable(0).getObjectAndCast<Container>();
      if (!container)
        continue;
      size_t containersLength = container->getNumElements();

      for (size_t position = 0; position < containersLength; ++position)
      {
        ObjectPtr subExample = new DenseGenericObject(inputsClass);
        size_t numSubInputs = example->getNumVariables();
        jassert(inputsClass->getNumMemberVariables() == numSubInputs);
        
        for (size_t input = 0; input < numSubInputs; ++input)
        {
          ContainerPtr container = example->getVariable(input).getObjectAndCast<Container>();
          if (container)
            subExample->setVariable(input, container->getElement(position));
        }
        subExamples[index++] = subExample;
      }
    }
    return res;
  }

  size_t computeNumSubExamples(const std::vector<ObjectPtr>& inputs) const
  {
    size_t res = 0;
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      ContainerPtr container = inputs[i]->getVariable(0).getObjectAndCast<Container>();
      if (container)
        res += container->getNumElements();
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_MAP_CONTAINER_FUNCTION_H_
