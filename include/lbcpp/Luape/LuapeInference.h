/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.h               | Luape Inference                 |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_INFERENCE_H_
# define LBCPP_LUAPE_INFERENCE_H_

# include "predeclarations.h"
# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class LuapeInference : public ExpressionDomain
{
public:
  LuapeInference(const ExpressionUniversePtr& universe = ExpressionUniversePtr())
    : ExpressionDomain(universe) {}

  /*
  ** Samples cache
  */
  virtual void setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData = std::vector<ObjectPtr>());

  const LuapeSamplesCachePtr& getTrainingCache() const
    {return trainingCache;}

  const LuapeSamplesCachePtr& getValidationCache() const
    {return validationCache;}

  std::vector<LuapeSamplesCachePtr> getSamplesCaches() const;

  VectorPtr getTrainingPredictions() const;
  VectorPtr getTrainingSupervisions() const;
  VectorPtr getValidationPredictions() const;
  VectorPtr getValidationSupervisions() const;

  /*
  ** Root node
  */
  const ExpressionPtr& getRootNode() const
    {return node;}
  void setRootNode(ExecutionContext& context, const ExpressionPtr& node);
  void clearRootNode(ExecutionContext& context);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
    {jassert(false); return 0.0;}
//void setLearner(const LuapeLearnerPtr& learner, bool verbose = false);

protected:
  ExpressionPtr node;
  LuapeSamplesCachePtr trainingCache;
  LuapeSamplesCachePtr validationCache;

  Variable computeNode(ExecutionContext& context, const ObjectPtr& inputs) const;

  LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;

// votes = DenseDoubleVector whose size is 2 x numYields (neg,pos,neg,pos ...)
class LuapeRegressor : public LuapeInference
{
public:
  LuapeRegressor(ExpressionUniversePtr universe = ExpressionUniversePtr())
    : LuapeInference(universe) {}

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;
extern ClassPtr luapeRegressorClass;

// votes = DenseDoubleVector of size numYields (symmetric weak predictor)
class LuapeBinaryClassifier : public LuapeInference
{
public:
  LuapeBinaryClassifier(ExpressionUniversePtr universe = ExpressionUniversePtr())
    : LuapeInference(universe) {}

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;
};

extern ClassPtr luapeBinaryClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeBinaryClassifier> LuapeBinaryClassifierPtr;

// votes = ObjectVector of size numYields, containing DenseDoubleVector of size numLabels (symmetric weak predictor)
class LuapeClassifier : public LuapeInference
{
public:
  LuapeClassifier(ExpressionUniversePtr universe = ExpressionUniversePtr())
    : LuapeInference(universe) {}

  static EnumerationPtr getLabelsFromSupervision(TypePtr supervisionType);

  const EnumerationPtr& getLabels() const
    {return labels;}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

  DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const ObjectPtr& input) const;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;

protected:
  ClassPtr doubleVectorClass;
  EnumerationPtr labels;
};

extern ClassPtr luapeClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeClassifier> LuapeClassifierPtr;

// votes = DenseDoubleVector whose size is numYields (symmetric weak learners)
class LuapeRanker : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual void setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData);
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;

  const std::vector<size_t>& getTrainingExampleSizes() const
    {return trainingExampleSizes;}

  const std::vector<size_t>& getValidationExampleSizes() const
    {return trainingExampleSizes;}

protected:
  std::vector<size_t> trainingExampleSizes;
  std::vector<size_t> validationExampleSizes;

  LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data, std::vector<size_t>& exampleSizes) const;
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_
