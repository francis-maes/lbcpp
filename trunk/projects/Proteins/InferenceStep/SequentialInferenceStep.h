/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceStep.h      | Base class for sequential       |
| Author  : Francis Maes                   |   inference                     |
| Started : 08/04/2010 18:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_SEQUENTIAL_H_
# define LBCPP_INFERENCE_STEP_SEQUENTIAL_H_

# include "InferenceStep.h"

namespace lbcpp
{

class CompositeInferenceStep : public InferenceStep
{
public:
  CompositeInferenceStep(const String& name) : InferenceStep(name) {}
  CompositeInferenceStep() {}

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

class SequentialInferenceStep : public CompositeInferenceStep
{
public:
  SequentialInferenceStep(const String& name) : CompositeInferenceStep(name) {}
  SequentialInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SequentialInferenceStepPtr(this));}

  virtual size_t getNumSubSteps() const = 0;
  virtual InferenceStepPtr getSubStep(size_t index) const = 0;
/*
  virtual bool saveToFile(const File& file) const
  {
    if (!saveToDirectory(file))
      return false;
    for (size_t i = 0; i < getNumSubSteps(); ++i)
    {
      InferenceStepPtr step = getSubStep(i);
      step->saveToFile(file.getChildFile(step->getName() + T(".inference")));
    }
    return true;
  }

  virtual bool loadFromFile(const File& file)
  {
    if (!loadFromDirectory(file))
      return false;
    for (size_t i = 0; i < getNumSubSteps(); ++i)
    {
      InferenceStepPtr step = getSubStep(i);
      jassert(step);
      if (!step->loadFromFile(file.getChildFile(step->getName() + T(".inference"))))
        return false;
    }
    return true;
  }*/
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_SEQUENTIAL_H_
