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

class ScalarInferenceLearner : public InferenceCallback
{
public:
  ScalarInferenceLearner(LinearScalarInferencePtr step)
    : step(step), epoch(1) {}

  // epoch starts at 1
  virtual bool learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr loss) = 0;

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (step == stack->getCurrentInference() && supervision && output)
    {
      FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
      ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
      ScalarPtr prediction = output.dynamicCast<Scalar>();
      jassert(features && loss && prediction);
      if (learningEpoch(epoch, features, prediction->getValue(), loss))
        ++epoch;
    }
  }

  virtual void finishInferencesCallback()
  {
    std::cout << "Epoch " << epoch << ", " << step->getParameters()->l0norm() << " parameters, L2 = " << step->getParameters()->l2norm() << std::endl;
  }

protected:
  LinearScalarInferencePtr step;
  size_t epoch;
};

class StochasticScalarLinearInferenceLearner : public ScalarInferenceLearner
{
public:
  StochasticScalarLinearInferenceLearner(LinearScalarInferencePtr step, IterationFunctionPtr learningRate, ScalarFunctionPtr regularizer = ScalarFunctionPtr(), bool normalizeLearningRate = true)
    : ScalarInferenceLearner(step), learningRate(learningRate), regularizer(regularizer), normalizeLearningRate(normalizeLearningRate) {}

  virtual bool learningEpoch(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr exampleLoss)
  {
    // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
    if (inputSize.getCount() < 10 ||                         // every time until having 10 samples
        (inputSize.getCount() < 100 && (epoch % 10 == 0)) || // every 10 epochs until having 100 samples
        (epoch % 100 == 0))                                  // every 100 epochs after that
    {
      inputSize.push((double)(features->l1norm()));
      //std::cout << "Alpha: " << weight * computeAlpha() << " inputSize: " << inputSize.toString() << std::endl;
    }

    if (exampleLoss->compute(1.0) > exampleLoss->compute(-1.0) && RandomGenerator::getInstance().sampleBool(0.95))
      return true; // reject 95% of negative examples

    //std::cout << "Loss: " << exampleLoss->toString() << " Derivative: " << exampleLoss->computeDerivative(prediction) << " PRediction = " << prediction << std::endl;
    double k = computeAlpha() * exampleLoss->computeDerivative(prediction);

    features->addWeightedTo(step->getParameters(), - k);
    step->clearDotProductCache();

    //if (epoch % 1000 == 0)
    //  std::cout << step->getParameters()->toString() << std::endl;

/*    size_t regularizerFrequency = 100;
    if ((epoch % regularizerFrequency) == 0)
    {
      // todo: apply regularizer
    }*/
    return true;
  }

protected:
  IterationFunctionPtr learningRate;
  ScalarFunctionPtr regularizer;
  bool normalizeLearningRate;
  ScalarVariableMean inputSize;

  double computeAlpha() const
  {
    double res = 1.0;
    if (learningRate)
      res *= learningRate->compute(epoch);
    if (normalizeLearningRate && inputSize.getMean())
      res /= inputSize.getMean();
    return res;
  }
};

/////////////////////////

class ProteinContactMapInferenceStep : public Protein2DInferenceStep
{
public:
  ProteinContactMapInferenceStep(const String& name, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DInferenceStep(name, InferencePtr(), features, targetName)
  {
    setSharedInferenceStep(new LinearScalarInference(name + T(" Classification")));
    //setSharedInferenceStep(new RegressionInferenceStep(name + T(" Classification")));
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
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
