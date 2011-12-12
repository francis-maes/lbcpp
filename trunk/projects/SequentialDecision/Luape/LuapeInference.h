/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.h               | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_INFERENCE_H_
# define LBCPP_LUAPE_INFERENCE_H_

# include "LuapeNode.h"

namespace lbcpp
{

class LuapeInference : public Function
{
public:
  LuapeInference();

  const LuapeNodeUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const LuapeInputNodePtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<LuapeInputNodePtr>& getInputs() const
    {return inputs;}

  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new LuapeInputNode(type, name));}

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const LuapeFunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const LuapeFunctionPtr& function)
    {functions.push_back(function);}

  /*
  ** Luape Node 
  */
  const LuapeNodePtr& getRootNode() const
    {return node;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const = 0;

protected:
  friend class LuapeInferenceClass;

  LuapeNodeUniversePtr universe;
  std::vector<LuapeInputNodePtr> inputs;
  std::vector<LuapeFunctionPtr> functions;
  LuapeNodePtr node;

  Variable computeNode(ExecutionContext& context, const ObjectPtr& inputs) const;
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
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

extern ClassPtr luapeBinaryClassifierClass;

// votes = ObjectVector of size numYields, containing DenseDoubleVector of size numLabels (symmetric weak predictor)
class LuapeClassifier : public LuapeInference
{
public:
  EnumerationPtr getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

  DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const ObjectPtr& input) const;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
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

  virtual LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_
