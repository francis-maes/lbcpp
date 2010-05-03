/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInferenceStep.h| Protein Contact Map Inference  |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_

# include "Protein2DInferenceStep.h"

namespace lbcpp
{

// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class ScalarLinearInferenceStep : public InferenceStep
{
public:
  ScalarLinearInferenceStep(const String& name)
    : InferenceStep(name), dotProductCache(NULL) {}

  virtual ~ScalarLinearInferenceStep()
    {clearDotProductCache();}

  virtual void accept(InferenceVisitorPtr visitor)
    {/*visitor->visit(InferenceStepPtr(this));*/}

  void createDotProductCache()
  {
    clearDotProductCache();
    dotProductCache = new FeatureGenerator::DotProductCache();
  }

  void clearDotProductCache()
  {
    delete dotProductCache;
    dotProductCache = NULL;
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

typedef ReferenceCountedObjectPtr<ScalarLinearInferenceStep> ScalarLinearInferenceStepPtr;

///////////////////////// not yet used /////////////////////////

class ScalarInferenceLearner : public InferenceCallback
{
public:
  ScalarInferenceLearner(ScalarLinearInferenceStepPtr step)
    : step(step), epoch(0) {}

  // epoch starts at 1
  virtual void learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr loss) = 0;

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (step == stack->getCurrentInference() && supervision && output)
    {
      FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      ScalarPtr prediction = output.dynamicCast<Scalar>();
      jassert(features && loss && prediction);
      learningEpoch(++epoch, features, prediction->getValue(), loss);
    }
  }

protected:
  ScalarLinearInferenceStepPtr step;
  size_t epoch;
};

class StochasticScalarLinearInferenceLearner : public ScalarInferenceLearner
{
public:
  StochasticScalarLinearInferenceLearner(ScalarLinearInferenceStepPtr step, IterationFunctionPtr learningRate, ScalarFunctionPtr regularizer = ScalarFunctionPtr())
    : ScalarInferenceLearner(step), learningRate(learningRate), regularizer(regularizer) {}

  virtual void learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr exampleLoss)
  {
    double alpha = learningRate->compute(epoch);
    features->addWeightedTo(step->getParameters(), -alpha * exampleLoss->computeDerivative(prediction));

    size_t regularizerFrequency = 100;
    if ((epoch % regularizerFrequency) == 0)
    {
      // todo: apply regularizer
    }
  }

protected:
  IterationFunctionPtr learningRate;
  ScalarFunctionPtr regularizer;
};

/////////////////////////

class ProteinContactMapInferenceStep : public Protein2DInferenceStep
{
public:
  ProteinContactMapInferenceStep(const String& name, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DInferenceStep(name, InferenceStepPtr(), features, targetName)
  {
    //ScalarLinearInferenceStepPtr sharedStep = new ScalarLinearInferenceStep(name + T(" Classification"));   
    setSharedInferenceStep(new RegressionInferenceStep(name + T(" Classification")));
  }

  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
  {
    size_t n = protein->getLength();
    res.reserve(n * (n - 5) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
        res.push_back(std::make_pair(i, j));
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
  {
    ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
    jassert(contactMap);
    if (!contactMap->hasScore(firstPosition, secondPosition))
      return ObjectPtr();
    return hingeLoss(contactMap->getScore(firstPosition, secondPosition) > 0.5 ? 1 : 0);
  }

  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
  {
    ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
    ScalarPtr score = subOutput.dynamicCast<Scalar>();
    jassert(contactMap && score);
    contactMap->setScore(firstPosition, secondPosition, 1.0 / (1.0 + exp(-score->getValue())));
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    RegressionInferenceStepPtr sharedStep = getSharedInferenceStep().dynamicCast<RegressionInferenceStep>();
    jassert(sharedStep);
    sharedStep->createDotProductCache();
    ObjectPtr res = Protein2DInferenceStep::run(context, input, supervision, returnCode);
    sharedStep->clearDotProductCache();
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
