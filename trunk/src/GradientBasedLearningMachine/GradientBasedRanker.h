/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedRanker.h          | Gradient based ranker           |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_RANKER_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_RANKER_H_

# include "StaticToDynamicGradientBasedMachine.h"

namespace lbcpp
{

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

class LargeMarginAllPairsLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginAllPairsLinearRanker>
{
public:
  inline impl::AllPairsLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::allPairsLoss<impl::HingeLossFunction, RankingExample>();}
};

class LargeMarginBestAgainstAllLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginBestAgainstAllLinearRanker>
{
public:
  inline impl::BestAgainstAllLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::bestAgainstAllLoss<impl::HingeLossFunction, RankingExample>();}
};

class LargeMarginMostViolatedPairLinearRanker
  : public StaticToDynamicGradientBasedLinearRanker<LargeMarginMostViolatedPairLinearRanker>
{
public:
  inline impl::MostViolatedPairLoss<impl::HingeLossFunction, RankingExample> loss() const
    {return impl::mostViolatedPairLoss<impl::HingeLossFunction, RankingExample>();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_RANKER_H_
