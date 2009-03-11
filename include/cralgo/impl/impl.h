/*-----------------------------------------.---------------------------------.
| Filename: impl.h                         | global include file for cralgo  |
| Author  : Francis Maes                   |                implementations  |
| Started : 05/01/2009 01:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_IMPL_H_
# define CRALGO_IMPL_H_

# include "Function/ScalarFunctions.hpp"
# include "Function/ScalarVectorFunctions.hpp"
# include "Function/PerceptronLossFunction.hpp"
# include "Function/HingeLossFunction.hpp"
# include "Function/LogBinomialLossFunction.hpp"
# include "Function/ExponentialLossFunction.hpp"
# include "Function/MulticlassLogBinomialLossFunction.hpp"
# include "Function/LinearArchitecture.hpp"
# include "Function/MultiLinearArchitecture.hpp"
# include "Function/BiasArchitecture.hpp"
# include "Function/TransferArchitecture.hpp"
# include "Function/EmpiricalRiskFunction.hpp"
# include "Function/ExampleRiskFunction.hpp"
# include "Function/FunctionStaticToDynamic.hpp"

# include "Policy/PolicyStaticToDynamic.hpp"
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
