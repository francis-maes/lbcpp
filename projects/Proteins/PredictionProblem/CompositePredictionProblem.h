/*-----------------------------------------.---------------------------------.
| Filename: CompositePredictionProblem.h   | Base class for composite        |
| Author  : Francis Maes                   |   prediction problems           |
| Started : 08/04/2010 18:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_
# define LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_

# include "PredictionProblem.h"

namespace lbcpp
{

class CompositePredictionProblem : public PredictionProblem
{
public:
  CompositePredictionProblem(const String& name) : PredictionProblem(name) {}

  virtual size_t getNumSubProblems() const = 0;
  virtual PredictionProblemPtr getSubProblem(size_t index) const = 0;

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
  }

};


class ChainPredictionProblem : public CompositePredictionProblem
{
public:
  ChainPredictionProblem(const String& name) : CompositePredictionProblem(name) {}

  virtual size_t getNumSubProblems() const
    {return subProblems.size();}

  virtual PredictionProblemPtr getSubProblem(size_t index) const
    {return subProblems[index];}

  void append(PredictionProblemPtr subProblem)
    {subProblems.push_back(subProblem);}

protected:
  std::vector<PredictionProblemPtr> subProblems;
};

typedef ReferenceCountedObjectPtr<ChainPredictionProblem> ChainPredictionProblemPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_COMPOSITE_PREDICTION_PROBLEM_H_
