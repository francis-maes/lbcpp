/*-----------------------------------------.---------------------------------.
| Filename: RankingBasedClassificationIn..h| Ranking-based Classification    |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2010 16:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_REDUCTION_RANKING_BASED_CLASSIFICATION_H_
# define LBCPP_INFERENCE_REDUCTION_RANKING_BASED_CLASSIFICATION_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class RankingBasedClassificationInference : public StaticDecoratorInference
{
public:
  RankingBasedClassificationInference(const String& name, InferencePtr rankingInference, EnumerationPtr classes)
    : StaticDecoratorInference(name, rankingInference), classes(classes)
  {
  }
  RankingBasedClassificationInference() {}

  virtual TypePtr getInputType() const
    {return decorated->getInputType()->getTemplateArgument(0)->getTemplateArgument(0);}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return classes;}
  
  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    TypePtr alternativeType = pairClass(input.getType(), classes);
    size_t n = classes->getNumElements();
    ContainerPtr alternatives = vector(alternativeType, n);
    for (size_t i = 0; i < n; ++i)
      alternatives->setElement(i, Variable::pair(input, Variable(i, classes), alternativeType));
    ContainerPtr costs;
    if (supervision.exists())
    {
      size_t correctClass = (size_t)supervision.getInteger();
      costs = vector(doubleType, n);
      for (size_t i = 0; i < n; ++i)
        costs->setElement(i, i == correctClass ? 0.0 : 1.0);
    }
    res->setSubInference(decorated, alternatives, costs);
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState) const
  {
    // todo: output discrete distribution
    const ContainerPtr& scores = finalState->getSubOutput().getObjectAndCast<Container>(context);
    if (!scores)
      return Variable::missingValue(classes);
    size_t n = classes->getNumElements();
    jassert(n == classes->getNumElements());
    double bestScore = -DBL_MAX;
    size_t res = 0;
    for (size_t i = 0; i < n; ++i)
    {
      double score = scores->getElement(i).getDouble();
      if (score > bestScore)
        bestScore = score, res = i;
    }
    return Variable(res, classes);
  }

private:
  friend class RankingBasedClassificationInferenceClass;

  EnumerationPtr classes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_RANKING_BASED_CLASSIFICATION_H_
