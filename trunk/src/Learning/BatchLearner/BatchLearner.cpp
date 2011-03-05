/*-----------------------------------------.---------------------------------.
| Filename: BatchLearner.cpp               | Batch Learners                  |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2011 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Learning/BatchLearner.h>
using namespace lbcpp;

Variable BatchLearner::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const FunctionPtr& function = inputs[0].getObjectAndCast<Function>();
  ObjectVectorPtr trainingData = makeObjectVector(inputs[1].getObjectAndCast<Container>());
  ObjectVectorPtr validationData = makeObjectVector(getNumInputs() == 3 ? inputs[2].getObjectAndCast<Container>() : ContainerPtr());
  jassert(!function->learning);
  function->learning = true;
  Variable res = train(context, function, trainingData->getObjects(), validationData ? validationData->getObjects() : std::vector<ObjectPtr>());
  function->learning = false;
  return res;
}

ObjectVectorPtr BatchLearner::makeObjectVector(const ContainerPtr& container)
{
  if (!container)
    return ObjectVectorPtr();
  ObjectVectorPtr res = container.dynamicCast<ObjectVector>();
  if (!res)
  {
    size_t n = container->getNumElements();
    res = new ObjectVector(container->getElementsType(), n);
    std::vector<ObjectPtr>& objects = res->getObjects();
    for (size_t i = 0; i < n; ++i)
      objects[i] = container->getElement(i).getObject();
  }
  return res;
}

bool BatchLearner::trainSubFunction(ExecutionContext& context, const FunctionPtr& subFunction, const ContainerPtr& subTrainingData, const ContainerPtr& subValidationData)
{
  String description = T("Learning ") + subFunction->getOutputVariable()->getName() + T(" with ");
  description += String((int)subTrainingData->getNumElements()) + T(" train examples");
  if (subValidationData)
    description += T(" and ") + String((int)subValidationData->getNumElements()) + T(" validation examples");
  return subFunction->train(context, subTrainingData, subValidationData, description);
}
