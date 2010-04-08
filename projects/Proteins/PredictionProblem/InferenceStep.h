/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.h            | Prediction problem base class   |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 18:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PREDICTION_PROBLEM_H_
# define LBCPP_PREDICTION_PROBLEM_H_

# include "CommonObjectFunctions.h"

namespace lbcpp
{

class ObjectPair : public Object
{
public:
  ObjectPair(ObjectPtr first, ObjectPtr second)
    : first(first), second(second) {}
  ObjectPair() {}

  ObjectPtr getFirst() const
    {return first;}

  ObjectPtr getSecond() const
    {return second;}

protected:
  ObjectPtr first;
  ObjectPtr second;
};

class ObjectPairContainer : public ObjectContainer
{
public:
  virtual String getFirstClassName() const
    {return T("Object");}

  virtual String getSecondClassName() const
    {return T("Object");}

  virtual std::pair<ObjectPtr, ObjectPtr> getPair(size_t index) const = 0;

  // ObjectContainer
  virtual String getContentClassName() const
    {return T("ObjectPair");}

  virtual ObjectPtr get(size_t index) const
  {
    std::pair<ObjectPtr, ObjectPtr> p = getPair(index);
    return new ObjectPair(p.first, p.second);
  }
};
typedef ReferenceCountedObjectPtr<ObjectPairContainer> ObjectPairContainerPtr;

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name) : NameableObject(name) {}

  ObjectFunctionPtr trainPredictor(const File& model, ObjectContainerPtr trainingData, const Time& lastDataModificationTime) const
  {
    if (isModelUpToDate(model, lastDataModificationTime))
      return loadPredictor(model);
    else
      return updatePredictor(model, trainingData);
  }

  virtual bool isModelUpToDate(const File& model, const Time& lastDataModificationTime) const = 0;
  virtual ObjectFunctionPtr loadPredictor(const File& model) const = 0;
  virtual ObjectFunctionPtr updatePredictor(const File& model, ObjectContainerPtr trainingData) const = 0; 
};

typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;

class AtomicInferenceStep : public InferenceStep
{
public:
  AtomicInferenceStep(const String& name) : InferenceStep(name) {}

  virtual ObjectFunctionPtr trainPredictor(ObjectContainerPtr trainingData) const = 0;

protected:
  virtual bool isModelUpToDate(const File& model, const Time& lastDataModificationTime) const
    {return model.existsAsFile() && model.getLastModificationTime() >= lastDataModificationTime;}

  virtual ObjectFunctionPtr loadPredictor(const File& model) const
    {return Object::loadFromFileAndCast<ObjectFunction>(model);}

  virtual ObjectFunctionPtr updatePredictor(const File& model, ObjectContainerPtr trainingData) const
  {
    ObjectFunctionPtr res = trainPredictor(trainingData);
    res->saveToFile(model);
    return res;
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_PREDICTION_PROBLEM_H_
