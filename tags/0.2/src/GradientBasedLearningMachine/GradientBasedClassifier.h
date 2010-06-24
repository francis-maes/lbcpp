/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedClassifier.h      | Gradient based classifier       |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_

# include "StaticToDynamicGradientBasedMachine.h"

namespace lbcpp
{

class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  virtual void setLabels(FeatureDictionaryPtr labels)
    {architecture_.setOutputs(labels); GradientBasedClassifier::setLabels(labels);}
  
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return architecture_;}

  inline impl::MultiClassLogBinomialLoss<ClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<ClassificationExample>();}

private:
  impl::MultiLinearArchitecture architecture_;
};

namespace impl
{

  struct MultiClassLargeMarginLossFunction : public ScalarVectorFunction< MultiClassLargeMarginLossFunction >
  {
    MultiClassLargeMarginLossFunction() : correctClass(-1) {}
    
    enum {isDerivable = true};
      
    void setCorrectClass(size_t correctClass)
      {this->correctClass = (int)correctClass;}
      
    void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, FeatureGeneratorPtr* gradient) const
    {
      jassert(correctClass >= 0);
      DenseVectorPtr scores = input->toDenseVector();
      if (!scores || !scores->getNumValues())
      {
        std::cerr << "No Scores, input = " << input->getClassName()
                  << " inputDictionary = " << input->getDictionary()->getName()
                  << " inputToString = " << input->toString() << std::endl;
        jassert(false);
      }
      jassert(scores && scores->getNumValues());
      
      double correctClassScore = scores->get(correctClass);

      impl::HingeLossFunction hingeLoss;

      if (output)
        *output = 0.0;
      DenseVectorPtr gradientVector;
      if (gradient)
        *gradient = (gradientVector = new DenseVector(scores->getDictionary(), scores->getNumValues()));

      double correctClassDerivative = 0.0;
      for (size_t i = 0; i < scores->getNumValues(); ++i)
      {
        if (i == (size_t)correctClass)
          continue;

        double margin = correctClassScore - scores->get(i);

        double hingeLossOutput, hingeLossDerivative;
        hingeLoss.compute(margin, output ? &hingeLossOutput : NULL, NULL, gradient ? &hingeLossDerivative : NULL);
        if (output)
          *output += hingeLossOutput;
        if (gradient)
        {
          correctClassDerivative += hingeLossDerivative;
          gradientVector->set(i, -hingeLossDerivative);
        }
      }
      if (gradient)
        gradientVector->set(correctClass, correctClassDerivative);
    }
    
  private:
    int correctClass;
  };

  inline MultiClassLargeMarginLossFunction multiClassLargeMarginLossFunction()
    {return MultiClassLargeMarginLossFunction();}

  STATIC_MULTICLASS_LOSS_FUNCTION(multiClassLargeMarginLoss, MultiClassLargeMarginLoss, MultiClassLargeMarginLossFunction);

}; /* namespace impl */

class LargeMarginClassifier
  : public StaticToDynamicGradientBasedLearningMachine<LargeMarginClassifier, GradientBasedClassifier>
{
public:
  virtual void setLabels(FeatureDictionaryPtr labels)
    {architecture_.setOutputs(labels); GradientBasedClassifier::setLabels(labels);}
  
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return architecture_;}

  inline impl::MultiClassLargeMarginLoss<ClassificationExample> loss() const
    {return impl::multiClassLargeMarginLoss<ClassificationExample>();}

private:
  impl::MultiLinearArchitecture architecture_;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_CLASSIFIER_H_
