/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearningMachine.cpp| Gradient based                 |
| Author  : Francis Maes                   |        learning machines        |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/GradientBasedLearningMachine.h>
#include <lbcpp/impl/impl.h>
using namespace lbcpp;

template<class ExactType, class BaseClass>
class StaticToDynamicGradientBasedLearningMachine : public BaseClass
{
public:
  typedef typename BaseClass::ExampleType ExampleType;
  
  // abstract: static functions for architecture() and loss()
  
  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const
    {return impl::staticToDynamic(impl::exampleRisk(_this().architecture(), _this().loss(), *example.staticCast<ExampleType>()));}
    
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const
    {return impl::staticToDynamic(impl::empiricalRisk(_this().architecture(), _this().loss(), examples, (ExampleType* )0));}
    
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
  {
    if (BaseClass::regularizer)
      return impl::staticToDynamic(impl::add(impl::empiricalRisk(_this().architecture(), _this().loss(), examples, (ExampleType* )0),
          impl::dynamicToStatic(BaseClass::regularizer)));
    else
      return getEmpiricalRisk(examples);
  }

protected:
  const ExactType& _this() const  {return *(const ExactType* )this;}
};

template<class ExactType>
class StaticToDynamicGradientBasedBinaryClassifier
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().architecture());}
};

template<class ExactType>
class StaticToDynamicGradientBasedGeneralizedClassifier
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedGeneralizedClassifier>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedGeneralizedClassifier> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().baseArchitecture());}
};

template<class ExactType>
class StaticToDynamicGradientBasedRanker
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedRanker>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedRanker> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().baseArchitecture());}
};

template<class ExactType>
class StaticToDynamicGradientBasedLinearRanker : public StaticToDynamicGradientBasedRanker<ExactType>
{
public:
  inline impl::LinearArchitecture baseArchitecture() const
    {return impl::linearArchitecture();}
  
  inline impl::ScalarToVectorArchitecture<impl::LinearArchitecture> architecture() const
    {return impl::parallelArchitecture(baseArchitecture());}
};

/*
** Regression
*/
class LeastSquaresLinearRegressor
  : public StaticToDynamicGradientBasedLearningMachine<LeastSquaresLinearRegressor, GradientBasedRegressor>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::SquareLoss<RegressionExample> loss() const
    {return impl::squareLoss<RegressionExample>();}
};

GradientBasedRegressorPtr GradientBasedRegressor::createLeastSquaresLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRegressorPtr res = new LeastSquaresLinearRegressor();
  res->setLearner(learner);
  return res;
}

/*
** Classifier
*/
class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  MaximumEntropyClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels)
    : architecture_(labels)
  {
    setLearner(learner);
    setLabels(labels);
  }
  
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return architecture_;}

  inline impl::MultiClassLogBinomialLoss<ClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<ClassificationExample>();}

private:
  impl::MultiLinearArchitecture architecture_;
};

GradientBasedClassifierPtr GradientBasedClassifier::createMaximumEntropy(GradientBasedLearnerPtr learner, StringDictionaryPtr labels)
{
  return new MaximumEntropyClassifier(learner, labels);
}

/*
** BinaryClassifier
*/
class LogisticRegressionClassifier : public StaticToDynamicGradientBasedBinaryClassifier<LogisticRegressionClassifier>
{
public:
  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::LogBinomialLoss<ClassificationExample> loss() const
    {return impl::logBinomialLoss<ClassificationExample>();}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLogisticRegression(GradientBasedLearnerPtr learner, StringDictionaryPtr labels)
{
  GradientBasedBinaryClassifierPtr res = new LogisticRegressionClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}

class LinearSupportVectorMachine : public StaticToDynamicGradientBasedBinaryClassifier<LinearSupportVectorMachine>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::HingeLoss<ClassificationExample> loss() const
    {return impl::hingeLoss<ClassificationExample>();}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLinearSVM(GradientBasedLearnerPtr learner, StringDictionaryPtr labels)
{
  GradientBasedBinaryClassifierPtr res = new LinearSupportVectorMachine();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}

/*
** Generalized classifier
*/
class LinearGeneralizedClassifier
  : public StaticToDynamicGradientBasedGeneralizedClassifier<LinearGeneralizedClassifier>
{
public:
  inline impl::LinearArchitecture baseArchitecture() const
    {return impl::linearArchitecture();}
  
  inline impl::ScalarToVectorArchitecture<impl::LinearArchitecture> architecture() const
    {return impl::parallelArchitecture(baseArchitecture());}

  inline impl::MultiClassLogBinomialLoss<GeneralizedClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<GeneralizedClassificationExample>();}
};

GradientBasedGeneralizedClassifierPtr GradientBasedGeneralizedClassifier::createLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedGeneralizedClassifierPtr res = new LinearGeneralizedClassifier();
  res->setLearner(learner);
  return res;
}

/*
** Ranker
*/
class LargeMarginAllPairsLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginAllPairsLinearRanker>
{
public:
  inline impl::AllPairsLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::allPairsLoss<impl::HingeLossFunction, RankingExample>();}
};

GradientBasedRankerPtr GradientBasedRanker::createLargeMarginAllPairsLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginAllPairsLinearRanker();
  res->setLearner(learner);
  return res;
}

class LargeMarginBestAgainstAllLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginBestAgainstAllLinearRanker>
{
public:
  inline impl::BestAgainstAllLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::bestAgainstAllLoss<impl::HingeLossFunction, RankingExample>();}
};

GradientBasedRankerPtr GradientBasedRanker::createLargeMarginBestAgainstAllLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginBestAgainstAllLinearRanker();
  res->setLearner(learner);
  return res;
}

class LargeMarginMostViolatedPairLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginMostViolatedPairLinearRanker>
{
public:
  inline impl::MostViolatedPairLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::mostViolatedPairLoss<impl::HingeLossFunction, RankingExample>();}
};

GradientBasedRankerPtr GradientBasedRanker::createLargeMarginMostViolatedPairLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginMostViolatedPairLinearRanker();
  res->setLearner(learner);
  return res;
}
