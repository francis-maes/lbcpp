/*-----------------------------------------.---------------------------------.
| Filename: PredictionProblem.h            | Prediction problem base class   |
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

class PredictionProblem : public NameableObject
{
public:
  PredictionProblem(const String& name) : NameableObject(name) {}

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

typedef ReferenceCountedObjectPtr<PredictionProblem> PredictionProblemPtr;

class AtomicPredictionProblem : public PredictionProblem
{
public:
  AtomicPredictionProblem(const String& name) : PredictionProblem(name) {}

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
