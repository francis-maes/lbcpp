/*-----------------------------------------.---------------------------------.
| Filename: impl.h                         | global include file for cralgo  |
| Author  : Francis Maes                   |                implementations  |
| Started : 05/01/2009 01:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_IMPL_H_
# define CRALGO_IMPL_H_

# include "ContinuousFunction/ScalarFunctions.hpp"
# include "ContinuousFunction/ScalarVectorFunctions.hpp"
# include "ContinuousFunction/PerceptronLossFunction.hpp"
# include "ContinuousFunction/HingeLossFunction.hpp"
# include "ContinuousFunction/LogBinomialLossFunction.hpp"
# include "ContinuousFunction/ExponentialLossFunction.hpp"
# include "ContinuousFunction/MulticlassLogBinomialLossFunction.hpp"
# include "ContinuousFunction/LinearArchitecture.hpp"
# include "ContinuousFunction/MultiLinearArchitecture.hpp"
# include "ContinuousFunction/BiasArchitecture.hpp"
# include "ContinuousFunction/TransferArchitecture.hpp"
# include "ContinuousFunction/EmpiricalRiskFunction.hpp"
# include "ContinuousFunction/ExampleRiskFunction.hpp"
# include "ContinuousFunction/FunctionStaticToDynamic.hpp"

# include "ChooseFunction/ChooseFunctionStaticToDynamic.hpp"
# include "ChooseFunction/ChooseFunctionDynamicToStatic.hpp"
# include "ChooseFunction/CompositeValueFunctions.hpp"
# include "ChooseFunction/ClassifierValueFunctions.hpp"
# include "ChooseFunction/RegressorValueFunctions.hpp"
# include "ChooseFunction/RankerValueFunctions.hpp"

# include "Policy/PolicyStaticToDynamic.hpp"
# include "Policy/PolicyDynamicToStatic.hpp"
# include "Policy/BasicPolicies.hpp"
# include "Policy/ComputeStatisticsPolicy.hpp"
# include "Policy/QLearningPolicy.hpp"
# include "Policy/MonteCarloControlPolicy.hpp"
# include "Policy/GPOMDPPolicy.hpp"
# include "Policy/ClassificationExampleCreatorPolicy.hpp"
# include "Policy/RankingExampleCreatorPolicy.hpp"
  
# include "Bridge/FeatureGenerator.hpp"
# include "Bridge/DoubleVector.hpp"
# include "Bridge/Choose.hpp"
# include "Bridge/Callback.hpp"
# include "Bridge/CRAlgorithmScope.hpp"
# include "Bridge/CRAlgorithm.hpp"

#endif // !CRALGO_IMPL_H_
