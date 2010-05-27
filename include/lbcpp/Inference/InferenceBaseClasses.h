/*-----------------------------------------.---------------------------------.
| Filename: InferenceBaseClasses.h         | Inference base classes          |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BASE_CLASSES_H_
# define LBCPP_INFERENCE_BASE_CLASSES_H_

# include "Inference.h"
# include "InferenceVisitor.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../FeatureGenerator/FeatureGenerator.h"
# include "../FeatureGenerator/DenseVector.h"
# include "../LearningMachine.h"
# include "../Object/ObjectPair.h"

namespace lbcpp
{

/*
** ParameterizedInference
*/
class ParameterizedInference : public Inference
{
public:
  ParameterizedInference(const String& name) : Inference(name) {}
  ParameterizedInference() {}

  virtual void accept(InferenceVisitorPtr visitor);

  virtual FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput, double& exampleLossValue) = 0;

  DenseVectorPtr getParameters() const
    {return parameters;}
 
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters; validateParametersChange();}

  virtual ObjectPtr clone() const;

  virtual void validateParametersChange() {}

protected:
  DenseVectorPtr parameters;

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

/*
** ParallelInference
*/
class ParallelInference : public Inference
{
public:
  ParallelInference(const String& name) : Inference(name) {}
  ParallelInference() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ParallelInferencePtr(this));}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInference(ParallelInferencePtr(this), input, supervision, returnCode);}

  virtual size_t getNumSubInferences(ObjectPtr input) const = 0;
  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const = 0;
};

class StaticParallelInference : public ParallelInference
{
public:
  StaticParallelInference(const String& name)
    : ParallelInference(name) {}
  StaticParallelInference() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(StaticParallelInferencePtr(this));}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getNumSubInferences();}

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return getSubInference(index);}
};

class SharedParallelInference : public ParallelInference
{
public:
  SharedParallelInference(const String& name, InferencePtr subInference);
  SharedParallelInference() {}

  InferencePtr getSharedInferenceStep() const
    {return subInference;}

  void setSharedInferenceStep(InferencePtr step)
    {subInference = step;}

  /*
  ** ParallelInference
  */
  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;

protected:
  InferencePtr subInference;
};

class VectorStaticParallelInference : public StaticParallelInference
{
public:
  VectorStaticParallelInference(const String& name)
    : StaticParallelInference(name) {}
  VectorStaticParallelInference() {}

  virtual size_t getNumSubInferences() const
    {return subInferences.size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences.get(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInferences.saveToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && subInferences.loadFromDirectory(file);}

protected:
  InferenceVector subInferences;
};

/*
** SequentialInference
*/
class SequentialInferenceState : public Object
{
public:
  SequentialInferenceState(ObjectPtr input, ObjectPtr supervision)
    : input(input), supervision(supervision), stepNumber(0) {}

  ObjectPtr getInput() const
    {return input;}

  ObjectPtr getSupervision() const
    {return supervision;}

  ObjectPtr getCurrentObject() const
    {return currentObject;}

  void setCurrentObject(ObjectPtr object)
    {currentObject = object;}

  size_t getCurrentStepNumber() const
    {return stepNumber;}

  void incrementStepNumber()
    {++stepNumber;}

  InferencePtr getCurrentSubInference() const
    {return subInference;}

  void setCurrentSubInference(InferencePtr subInference)
    {this->subInference = subInference;}

  bool isFinal() const
    {return !subInference;}

private:
  ObjectPtr input;
  ObjectPtr supervision;
  ObjectPtr currentObject;
  size_t stepNumber;
  InferencePtr subInference;
};
typedef ReferenceCountedObjectPtr<SequentialInferenceState> SequentialInferenceStatePtr;

class SequentialInference : public Inference
{
public:
  SequentialInference(const String& name) : Inference(name) {}
  SequentialInference() {}

  /*
  ** Abstract
  */
  virtual ObjectPtr prepareInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return state->getInput();}

  virtual InferencePtr getInitialSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const = 0;

  virtual ObjectPairPtr prepareSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return new ObjectPair(state->getCurrentObject(), state->getSupervision());}
  
  virtual ObjectPtr finalizeSubInference(SequentialInferenceStatePtr state, ObjectPtr subInferenceOutput, ReturnCode& returnCode) const
    {return subInferenceOutput;}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const = 0;

  virtual ObjectPtr finalizeInference(SequentialInferenceStatePtr finalState, ReturnCode& returnCode) const
    {return finalState->getCurrentObject();}

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SequentialInferencePtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runSequentialInference(SequentialInferencePtr(this), input, supervision, returnCode);}

  /*
  ** Object
  */
  virtual String toString() const;
};

class VectorSequentialInference : public SequentialInference
{
public:
  VectorSequentialInference(const String& name)
    : SequentialInference(name) {}

  virtual InferencePtr getInitialSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return subInferences.get(0);}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
  {
    size_t index = state->getCurrentStepNumber();
    jassert(state->getCurrentSubInference() == subInferences.get(index));
    ++index;
    return index < subInferences.size() ? subInferences.get(index) : InferencePtr();
  }

  size_t getNumSubInferences() const
    {return subInferences.size();}

  InferencePtr getSubInference(size_t index) const
    {return subInferences.get(index);}
 
  void appendInference(InferencePtr inference)
    {subInferences.append(inference);}

  File getSubInferenceFile(size_t index, const File& modelDirectory) const
    {return subInferences.getSubInferenceFile(index, modelDirectory);}

  bool loadSubInferencesFromDirectory(const File& file)
    {return subInferences.loadFromDirectory(file);}

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInferences.saveToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && subInferences.loadFromDirectory(file);}

protected:
  InferenceVector subInferences;
};

typedef ReferenceCountedObjectPtr<VectorSequentialInference> VectorSequentialInferencePtr;

/*
** DecoratorInference
*/
class DecoratorInference : public Inference
{
public:
  DecoratorInference(const String& name, InferencePtr decorated)
    : Inference(name), decorated(decorated) {}
  DecoratorInference() {}
 
  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;
  virtual ObjectPtr clone() const;

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  InferencePtr getDecoratedInference() const
    {return decorated;}

protected:
  InferencePtr decorated;
};

// Input: FeatureGenerator
// Output: FeatureVector
// Supervision: Label
class ClassificationInferenceStep : public Inference
{
public:
  ClassificationInferenceStep(const String& name)
    : Inference(name) {}
  ClassificationInferenceStep() {}

  virtual String toString() const
  {
    String res = getClassName();
    if (labels)
      res += T("(") + labels->getName() + T(")");
    return res;
  }

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ClassificationInferenceStepPtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runClassification(ClassificationInferenceStepPtr(this), input, supervision, returnCode);}

  ClassifierPtr getClassifier() const
    {return classifier;}

  void setClassifier(ClassifierPtr classifier)
    {this->classifier = classifier;}

  void setLabels(FeatureDictionaryPtr labels)
    {this->labels = labels;}

  FeatureDictionaryPtr getLabels() const
    {return labels;}

protected:
  FeatureDictionaryPtr labels;
  ClassifierPtr classifier;

  virtual bool load(InputStream& istr)
    {return Inference::load(istr) && lbcpp::read(istr, classifier);}

  virtual void save(OutputStream& ostr) const
    {Inference::save(ostr); lbcpp::write(ostr, classifier);}
};

// Input: FeatureGenerator
// Output: Scalar
// Supervision: Scalar
class RegressionInferenceStep : public Inference
{
public:
  RegressionInferenceStep(const String& name)
    : Inference(name) {}
  RegressionInferenceStep() {}

  virtual String toString() const
    {return getClassName();}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(RegressionInferenceStepPtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runRegression(RegressionInferenceStepPtr(this), input, supervision, returnCode);}

  RegressorPtr getRegressor() const
    {return regressor;}

  void setRegressor(RegressorPtr regressor)
    {this->regressor = regressor;}

protected:
  RegressorPtr regressor;

  virtual bool load(InputStream& istr)
    {return Inference::load(istr) && lbcpp::read(istr, regressor);}

  virtual void save(OutputStream& ostr) const
    {Inference::save(ostr); lbcpp::write(ostr, regressor);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BASE_CLASSES_H_
