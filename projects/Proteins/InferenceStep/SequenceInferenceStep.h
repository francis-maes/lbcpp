/*-----------------------------------------.---------------------------------.
| Filename: CompositeInferenceStep.h   | Base class for composite        |
| Author  : Francis Maes                   |   prediction problems           |
| Started : 08/04/2010 18:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_
# define LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_

# include "InferenceStep.h"

namespace lbcpp
{


class CompositeInferenceStep : public InferenceStep
{
public:
  CompositeInferenceStep(const String& name) : InferenceStep(name) {}
  CompositeInferenceStep() {}

  // returns a graph of interdependant InferenceSteps
  virtual ObjectGraphPtr getDependencyGraph() const = 0;

/*
protected:
  virtual bool isModelUpToDate(const File& model, const Time& lastDataModificationTime) const
    {return model.exists() && model.isDirectory() && model.getLastModificationTime() >= lastDataModificationTime;}

  virtual ObjectFunctionPtr loadPredictor(const File& model) const
  {
    if (!model.exists() || !model.isDirectory())
    {
      Object::error(T("loadPredictor"), model.getFullPathName() + T(" is not a directory"));
      return ObjectFunctionPtr();
    }
  
    // todo: load childs
    // create CompositePredictor
    return ObjectFunctionPtr();          
  }

  virtual void savePredictor(ObjectFunctionPtr predictor, const File& model) const
  {
    bool ok = model.createDirectory();
    if (!ok)
    {
      Object::error(T("savePredictor"), T("Could not create directory ") + model.getFullPathName());
      return;
    }

    // todo: save childs
  }
  
  virtual ObjectFunctionPtr updatePredictor(const File& model, ObjectContainerPtr trainingData) const
  {
    return ObjectFunctionPtr();
  }*/
};

class SequenceInferenceStep : public CompositeInferenceStep
{
public:
  SequenceInferenceStep(const String& name) : CompositeInferenceStep(name) {}
  SequenceInferenceStep() {}

  virtual ObjectGraphPtr getDependencyGraph() const
  {
    ObjectContainerPtr subStepsContainer = new VectorObjectContainer(*(std::vector<ObjectPtr>* )&subSteps, T("InferenceStep"));
    return subStepsContainer->toGraph();
  }

  virtual void accept(InferenceVisitorPtr visitor)
  {

  }
  size_t getNumSubSteps() const
    {return subSteps.size();}

  InferenceStepPtr getSubStep(size_t index) const
    {jassert(index < subSteps.size()); return subSteps[index];}

  void appendSubStep(InferenceStepPtr subStep)
    {subSteps.push_back(subStep);}

protected:
  std::vector<InferenceStepPtr> subSteps;
};

}; /* namespace lbcpp */

#endif //!LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_
