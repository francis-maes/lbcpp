/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.h               | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_INFERENCE_H_
# define LBCPP_LUAPE_INFERENCE_H_

# include "LuapeGraph.h"

namespace lbcpp
{

class LuapeInference : public Function
{
public:
  const LuapeGraphPtr& getGraph() const
    {return graph;}

  void setGraph(const LuapeGraphPtr& graph)
    {this->graph = graph;}

  const VectorPtr& getVotes() const
    {return votes;}

  void setVotes(const VectorPtr& votes)
    {this->votes = votes;}

  virtual VectorPtr createVoteVector() const = 0;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const = 0;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const = 0;

  virtual TypePtr getPredictionsInternalType() const
    {return getOutputType();}

  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);

  VectorPtr makeCachedPredictions(ExecutionContext& context, bool isTrainingSamples) const;
  DenseDoubleVectorPtr computeSignedWeakPredictions(ExecutionContext& context, const ObjectPtr& input) const;

protected:
  friend class LuapeInferenceClass;

  LuapeGraphPtr graph;
  VectorPtr votes;

  void computeGraph(ExecutionContext& context, const ObjectPtr& inputs, std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
};

typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;
extern ClassPtr luapeInferenceClass;

// votes = DenseDoubleVector whose size is 2 x numYields (neg,pos,neg,pos ...)
class LuapeRegressor : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;
  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;

// votes = DenseDoubleVector of size numYields (symmetric weak predictor)
class LuapeBinaryClassifier : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

extern ClassPtr luapeBinaryClassifierClass;

// votes = ObjectVector of size numYields, containing DenseDoubleVector of size numLabels (symmetric weak predictor)
class LuapeClassifier : public LuapeInference
{
public:
  const EnumerationPtr& getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

  DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const ObjectPtr& input) const;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual VectorPtr createVoteVector() const;
  virtual TypePtr getPredictionsInternalType() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;

protected:
  ClassPtr doubleVectorClass;
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

  // votes are scalars (alpha values)
  virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;
  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_
