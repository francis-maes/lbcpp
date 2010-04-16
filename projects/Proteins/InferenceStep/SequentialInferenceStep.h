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

class VectorSequentialInferenceStep : public SequentialInferenceStep
{
public:
  VectorSequentialInferenceStep(const String& name)
    : SequentialInferenceStep(name) {}

  void appendStep(InferenceStepPtr inference)
    {inferenceSteps.push_back(inference);}

 virtual size_t getNumSubSteps() const
    {return inferenceSteps.size();}

  virtual InferenceStepPtr getSubStep(size_t index) const
    {jassert(index < inferenceSteps.size()); return inferenceSteps[index];}

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

protected:
  std::vector<InferenceStepPtr> inferenceSteps;
/*
  virtual bool load(InputStream& istr)
  {
    size_t size;
    if (!lbcpp::read(istr, size))
      return false;
    inferenceSteps.resize(size);
    for (size_t i = 0; i < inferenceSteps.size(); ++i)
      if (!lbcpp::read(istr, inferenceSteps[i].second))
        return false;
    return true;
  }
  virtual void save(OutputStream& ostr) const
    {}*/
};
}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_SEQUENTIAL_H_
