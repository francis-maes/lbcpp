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

class VectorBasedInferenceHelper
{
public:
  size_t getNumSubSteps() const
    {return subInferences.size();}

  InferencePtr getSubStep(size_t index) const
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

class LearnableAtomicInference : public Inference
{
public:
  LearnableAtomicInference(const String& name) : Inference(name) {}
  LearnableAtomicInference() {}

  virtual void accept(InferenceVisitorPtr visitor);
};


// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearScalarInference : public LearnableAtomicInference
{
public:
  LinearScalarInference(const String& name)
    : LearnableAtomicInference(name), dotProductCache(NULL) {}

  virtual ~LinearScalarInference()
    {clearDotProductCache();}

  virtual void beginRunSession()
    {clearDotProductCache(); dotProductCache = new FeatureGenerator::DotProductCache();}

  virtual void endRunSession()
    {clearDotProductCache();}
  
  void clearDotProductCache()
  {
    if (dotProductCache)
    {
      delete dotProductCache;
      dotProductCache = NULL;
    }
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    if (!parameters)
    {
      parameters = new DenseVector(features->getDictionary());
      return new Scalar(0.0);
    }
    return new Scalar(features->dotProduct(parameters, dotProductCache));
  }

  DenseVectorPtr getParameters() const
    {return parameters;}

private:
  DenseVectorPtr parameters;
  FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearScalarInference> LinearScalarInferencePtr;

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

class SharedParallelInference : public ParallelInference
{
public:
  SharedParallelInference(const String& name, InferencePtr subInference)
    : ParallelInference(name), subInference(subInference) {}
  SharedParallelInference() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SharedParallelInferencePtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    subInference->beginRunSession();
    ObjectPtr res = ParallelInference::run(context, input, supervision, returnCode);
    subInference->endRunSession();
    return res;
  }

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

  virtual String toString() const
    {jassert(subInference); return getClassName() + T("(") + subInference->toString() + T(")");}

  virtual bool loadFromFile(const File& file)
  {
    if (!loadFromDirectory(file))
      return false;
    subInference = createFromFileAndCast<Inference>(file.getChildFile(T("shared.inference")));
    return subInference != InferencePtr();
  }

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInference->saveToFile(file.getChildFile(T("shared.inference")));}

  InferencePtr getSharedInferenceStep() const
    {return subInference;}

  void setSharedInferenceStep(InferencePtr step)
    {subInference = step;}

protected:
  InferencePtr subInference;
};


class ParallelSharedMultiRegressionInference : public SharedParallelInference
{
public:
  ParallelSharedMultiRegressionInference(const String& name, FeatureDictionaryPtr outputDictionary)
    : SharedParallelInference(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    return container->size();
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getInputFeatures(input, index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
  {
    if (!supervision)
      return ObjectPtr();
    DenseVectorPtr vector = supervision.dynamicCast<DenseVector>();
    jassert(vector);
    return new Scalar(vector->get(index));
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new DenseVector(outputDictionary, getNumSubInferences(input));}
  
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    DenseVectorPtr vector = output.dynamicCast<DenseVector>();
    jassert(vector);
    ScalarPtr scalar = subOutput.dynamicCast<Scalar>();
    jassert(scalar);
    vector->set(index, scalar->getValue());
  }

protected:
  FeatureDictionaryPtr outputDictionary;
};

class VectorParallelInference : public ParallelInference, public VectorBasedInferenceHelper
{
public:
  VectorParallelInference(const String& name)
    : ParallelInference(name) {}
  VectorParallelInference() {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return VectorBasedInferenceHelper::getNumSubSteps();}

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return VectorBasedInferenceHelper::getSubStep(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};


class SequentialInference : public Inference
{
public:
  SequentialInference(const String& name) : Inference(name) {}
  SequentialInference() {}

  /*
  ** Abstract
  */
  virtual size_t getNumSubSteps() const = 0;
  virtual InferencePtr getSubStep(size_t index) const = 0;

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

class VectorSequentialInference : public SequentialInference, public VectorBasedInferenceHelper
{
public:
  VectorSequentialInference(const String& name)
    : SequentialInference(name) {}

  virtual size_t getNumSubSteps() const
    {return VectorBasedInferenceHelper::getNumSubSteps();}

  virtual InferencePtr getSubStep(size_t index) const
    {return VectorBasedInferenceHelper::getSubStep(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};

typedef ReferenceCountedObjectPtr<VectorSequentialInference> VectorSequentialInferencePtr;


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

class CallbackBasedDecoratorInference : public DecoratorInference
{
public:
  CallbackBasedDecoratorInference(const String& name, InferencePtr decorated, InferenceCallbackPtr callback)
    : DecoratorInference(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInference() {}
 
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

protected:
  InferenceCallbackPtr callback;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class TransferRegressionInferenceStep : public DecoratorInference
{
public:
  TransferRegressionInferenceStep(const String& name, InferencePtr regressionStep, ScalarFunctionPtr transferFunction)
    : DecoratorInference(name, regressionStep), transferFunction(transferFunction) {}
  TransferRegressionInferenceStep() {}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      supervision = transferFunction->composeWith(loss);
    }
    ObjectPtr result = DecoratorInference::run(context, input, supervision, returnCode);
    if (result)
    {
      ScalarPtr scalarResult = result.dynamicCast<Scalar>();
      jassert(scalarResult);
      result = new Scalar(transferFunction->compute(scalarResult->getValue()));
    }
    return result;
  }

protected:
  ScalarFunctionPtr transferFunction;

  virtual bool load(InputStream& istr)
    {return DecoratorInference::load(istr) && lbcpp::read(istr, transferFunction);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInference::save(ostr); lbcpp::write(ostr, transferFunction);}
};


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BASE_CLASSES_H_
