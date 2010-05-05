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
class ClassificationInferenceStep : public InferenceStep
{
public:
  ClassificationInferenceStep(const String& name)
    : InferenceStep(name) {}
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
    {return InferenceStep::load(istr) && lbcpp::read(istr, classifier);}

  virtual void save(OutputStream& ostr) const
    {InferenceStep::save(ostr); lbcpp::write(ostr, classifier);}
};

// Input: FeatureGenerator
// Output: Scalar
// Supervision: Scalar
class RegressionInferenceStep : public InferenceStep
{
public:
  RegressionInferenceStep(const String& name)
    : InferenceStep(name) {}
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
    {return InferenceStep::load(istr) && lbcpp::read(istr, regressor);}

  virtual void save(OutputStream& ostr) const
    {InferenceStep::save(ostr); lbcpp::write(ostr, regressor);}
};

class VectorBasedInferenceHelper
{
public:
  size_t getNumSubSteps() const
    {return subInferences.size();}

  InferenceStepPtr getSubStep(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index];}

  void setSubStep(size_t index, InferenceStepPtr subStep)
    {jassert(index < subInferences.size()); subInferences[index] = subStep;}

  void appendStep(InferenceStepPtr inference)
    {subInferences.push_back(inference);}

  File getSubInferenceFile(size_t index, const File& directory) const;

  int findStepNumber(InferenceStepPtr step) const;

  bool saveSubInferencesToDirectory(const File& file) const;
  bool loadSubInferencesFromDirectory(const File& file);

protected:
  std::vector<InferenceStepPtr> subInferences;
};

class LearnableAtomicInferenceStep : public InferenceStep
{
public:
  LearnableAtomicInferenceStep(const String& name) : InferenceStep(name) {}
  LearnableAtomicInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor);
};


// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearScalarInferenceStep : public LearnableAtomicInferenceStep
{
public:
  LinearScalarInferenceStep(const String& name)
    : LearnableAtomicInferenceStep(name), dotProductCache(NULL) {}

  virtual ~LinearScalarInferenceStep()
    {clearDotProductCache();}

  void createDotProductCache()
  {
    clearDotProductCache();
    dotProductCache = new FeatureGenerator::DotProductCache();
  }

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

typedef ReferenceCountedObjectPtr<LinearScalarInferenceStep> LinearScalarInferenceStepPtr;

class ParallelInferenceStep : public InferenceStep
{
public:
  ParallelInferenceStep(const String& name) : InferenceStep(name) {}
  ParallelInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ParallelInferenceStepPtr(this));}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInferences(ParallelInferenceStepPtr(this), input, supervision, returnCode);}

  virtual size_t getNumSubInferences(ObjectPtr input) const = 0;
  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const = 0;
};

class SharedParallelInferenceStep : public ParallelInferenceStep
{
public:
  SharedParallelInferenceStep(const String& name, InferenceStepPtr subInference)
    : ParallelInferenceStep(name), subInference(subInference) {}
  SharedParallelInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SharedParallelInferenceStepPtr(this));}
  
  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

  virtual String toString() const
    {jassert(subInference); return getClassName() + T("(") + subInference->toString() + T(")");}

  virtual bool loadFromFile(const File& file)
  {
    if (!loadFromDirectory(file))
      return false;
    subInference = createFromFileAndCast<InferenceStep>(file.getChildFile(T("shared.inference")));
    return subInference != InferenceStepPtr();
  }

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInference->saveToFile(file.getChildFile(T("shared.inference")));}

  InferenceStepPtr getSharedInferenceStep() const
    {return subInference;}

  void setSharedInferenceStep(InferenceStepPtr step)
    {subInference = step;}

protected:
  InferenceStepPtr subInference;
};


class ParallelSharedMultiRegressionInferenceStep : public SharedParallelInferenceStep
{
public:
  ParallelSharedMultiRegressionInferenceStep(const String& name, FeatureDictionaryPtr outputDictionary)
    : SharedParallelInferenceStep(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

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

class VectorParallelInferenceStep : public ParallelInferenceStep, public VectorBasedInferenceHelper
{
public:
  VectorParallelInferenceStep(const String& name)
    : ParallelInferenceStep(name) {}
  VectorParallelInferenceStep() {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return VectorBasedInferenceHelper::getNumSubSteps();}

  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return VectorBasedInferenceHelper::getSubStep(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};


class SequentialInferenceStep : public InferenceStep
{
public:
  SequentialInferenceStep(const String& name) : InferenceStep(name) {}
  SequentialInferenceStep() {}

  /*
  ** Abstract
  */
  virtual size_t getNumSubSteps() const = 0;
  virtual InferenceStepPtr getSubStep(size_t index) const = 0;

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
    {return supervision;}

  /*
  ** Object
  */
  virtual String toString() const;

  /*
  ** InferenceStep
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
};

class VectorSequentialInferenceStep : public SequentialInferenceStep, public VectorBasedInferenceHelper
{
public:
  VectorSequentialInferenceStep(const String& name)
    : SequentialInferenceStep(name) {}

  virtual size_t getNumSubSteps() const
    {return VectorBasedInferenceHelper::getNumSubSteps();}

  virtual InferenceStepPtr getSubStep(size_t index) const
    {return VectorBasedInferenceHelper::getSubStep(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};

typedef ReferenceCountedObjectPtr<VectorSequentialInferenceStep> VectorSequentialInferenceStepPtr;


class DecoratorInferenceStep : public InferenceStep
{
public:
  DecoratorInferenceStep(const String& name, InferenceStepPtr decorated)
    : InferenceStep(name), decorated(decorated) {}
  DecoratorInferenceStep() {}
 
  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;

  /*
  ** InferenceStep
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  InferenceStepPtr getDecoratedInference() const
    {return decorated;}

protected:
  InferenceStepPtr decorated;
};

class CallbackBasedDecoratorInferenceStep : public DecoratorInferenceStep
{
public:
  CallbackBasedDecoratorInferenceStep(const String& name, InferenceStepPtr decorated, InferenceCallbackPtr callback)
    : DecoratorInferenceStep(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInferenceStep() {}
 
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

protected:
  InferenceCallbackPtr callback;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

class TransferRegressionInferenceStep : public DecoratorInferenceStep
{
public:
  TransferRegressionInferenceStep(const String& name, InferenceStepPtr regressionStep, ScalarFunctionPtr transferFunction)
    : DecoratorInferenceStep(name, regressionStep), transferFunction(transferFunction) {}
  TransferRegressionInferenceStep() {}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      jassert(loss);
      supervision = transferFunction->composeWith(loss);
    }
    ObjectPtr result = DecoratorInferenceStep::run(context, input, supervision, returnCode);
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
    {return DecoratorInferenceStep::load(istr) && lbcpp::read(istr, transferFunction);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInferenceStep::save(ostr); lbcpp::write(ostr, transferFunction);}
};


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BASE_CLASSES_H_
