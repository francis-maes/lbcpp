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

namespace lbcpp
{

/*
** AtomicInference
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
  
  virtual ObjectPtr clone() const;

  virtual void validateParametersChange() {}

protected:
  DenseVectorPtr parameters;

  DenseVectorPtr getOrCreateParameters(FeatureDictionaryPtr dictionary);

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
    {return context->runParallelInferences(ParallelInferencePtr(this), input, supervision, returnCode);}

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

class ParallelSharedMultiRegressionInference : public SharedParallelInference
{
public:
  ParallelSharedMultiRegressionInference(const String& name, FeatureDictionaryPtr outputDictionary);

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const;
  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const;

protected:
  FeatureDictionaryPtr outputDictionary;
};

/*
** SequentialInference
*/
class SequentialInference : public Inference
{
public:
  SequentialInference(const String& name) : Inference(name) {}
  SequentialInference() {}

  /*
  ** Abstract
  */
  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
    {return supervision;}

  /*
  ** Object
  */
  virtual String toString() const;

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
};

/*
** Inferences with a vector of sub-inferences
*/
class VectorBasedInferenceHelper
{
public:
  size_t getNumSubInferences() const
    {return subInferences.size();}

  InferencePtr getSubInference(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index];}

  void setSubStep(size_t index, InferencePtr subStep)
    {jassert(index < subInferences.size()); subInferences[index] = subStep;}

  void appendStep(InferencePtr inference)
    {subInferences.push_back(inference);}

  File getSubInferenceFile(size_t index, const File& directory) const;

  int findStepNumber(InferencePtr step) const;

  bool saveSubInferencesToDirectory(const File& file) const;
  bool loadSubInferencesFromDirectory(const File& file);

protected:
  std::vector<InferencePtr> subInferences;
};

class VectorStaticParallelInference : public StaticParallelInference, public VectorBasedInferenceHelper
{
public:
  VectorStaticParallelInference(const String& name)
    : StaticParallelInference(name) {}
  VectorStaticParallelInference() {}

  virtual size_t getNumSubInferences() const
    {return VectorBasedInferenceHelper::getNumSubInferences();}

  virtual InferencePtr getSubInference(size_t index) const
    {return VectorBasedInferenceHelper::getSubInference(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};

class VectorSequentialInference : public SequentialInference, public VectorBasedInferenceHelper
{
public:
  VectorSequentialInference(const String& name)
    : SequentialInference(name) {}

  virtual size_t getNumSubInferences() const
    {return VectorBasedInferenceHelper::getNumSubInferences();}

  virtual InferencePtr getSubInference(size_t index) const
    {return VectorBasedInferenceHelper::getSubInference(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
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

extern ParameterizedInferencePtr linearScalarInference(const String& name);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);
extern InferencePtr callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback);

extern InferencePtr binaryLinearSVMInference(const String& name = T("unnamed"));
extern InferencePtr binaryLogisticRegressionInference(const String& name = T("unnamed"));

extern InferencePtr oneAgainstAllClassificationInference(const String& name, FeatureDictionaryPtr labelsDictionary, InferencePtr binaryClassifierModel);

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
