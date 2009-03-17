/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearningMachine.cpp| Gradient based                 |
| Author  : Francis Maes                   |        learning machines        |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/GradientBasedLearningMachine.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

template<class ExactType, class BaseClass>
class StaticToDynamicGradientBasedLearningMachine : public BaseClass
{
public:
  typedef typename BaseClass::ExampleType ExampleType;
  
  // abstract: static functions for architecture(), loss() and regularizer()
  
  virtual ScalarVectorFunctionPtr getRegularizer() const
    {return impl::staticToDynamic(_this().regularizer());}
  
  virtual ScalarVectorFunctionPtr getLoss(const ExampleType& example) const
    {return impl::staticToDynamic(impl::exampleRisk(_this().architecture(), _this().loss(), example));}
    
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {return impl::staticToDynamic(impl::empiricalRisk(_this().architecture(), _this().loss(), examples));}
    
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {return impl::staticToDynamic(impl::add(impl::empiricalRisk(_this().architecture(), _this().loss(), examples), _this().regularizer()));}

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
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedRegressorPtr GradientBasedRegressor::createLeastSquaresLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRegressorPtr res = new LeastSquaresLinearRegressor();
  res->setLearner(learner);
  res->createParameters();
  return res;
}

/*
** Classifier
*/
class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return impl::multiLinearArchitecture(getLabels());}

  inline impl::MultiClassLogBinomialLoss<ClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<ClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0));}
};

GradientBasedClassifierPtr GradientBasedClassifier::createMaximumEntropy(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedClassifierPtr res = new MaximumEntropyClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  res->createParameters();
  return res;
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
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLogisticRegression(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedBinaryClassifierPtr res = new LogisticRegressionClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  res->createParameters();
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
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLinearSVM(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedBinaryClassifierPtr res = new LinearSupportVectorMachine();
  res->setLearner(learner);
  res->setLabels(labels);
  res->createParameters();
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
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.0));}
};

GradientBasedGeneralizedClassifierPtr GradientBasedGeneralizedClassifier::createLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedGeneralizedClassifierPtr res = new LinearGeneralizedClassifier();
  res->setLearner(learner);
  res->createParameters();
  return res;
}

/*
** Ranker
*/
class LargeMarginAllPairsLinearRanker
  : public StaticToDynamicGradientBasedRanker<LargeMarginAllPairsLinearRanker>
{
public:
  inline impl::LinearArchitecture baseArchitecture() const
    {return impl::linearArchitecture();}
  
  inline impl::ScalarToVectorArchitecture<impl::LinearArchitecture> architecture() const
    {return impl::parallelArchitecture(baseArchitecture());}

  inline impl::AllPairsLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::allPairsLoss<impl::HingeLossFunction, RankingExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.0));}
};

GradientBasedRankerPtr GradientBasedRanker::createLargeMarginAllPairsLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginAllPairsLinearRanker();
  res->setLearner(learner);
  res->createParameters();
  return res;
}

class LargeMarginBestAgainstAllLinearRanker
  : public StaticToDynamicGradientBasedRanker<LargeMarginBestAgainstAllLinearRanker>
{
public:
  inline impl::LinearArchitecture baseArchitecture() const
    {return impl::linearArchitecture();}
  
  inline impl::ScalarToVectorArchitecture<impl::LinearArchitecture> architecture() const
    {return impl::parallelArchitecture(baseArchitecture());}

  inline impl::BestAgainstAllLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::bestAgainstAllLoss<impl::HingeLossFunction, RankingExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.0));}
};

GradientBasedRankerPtr GradientBasedRanker::createLargeMarginBestAgainstAllLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRankerPtr res = new LargeMarginBestAgainstAllLinearRanker();
  res->setLearner(learner);
  res->createParameters();
  return res;
}
