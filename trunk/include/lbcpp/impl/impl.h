/*-----------------------------------------.---------------------------------.
| Filename: impl.h                         | global include file for lbcpp  |
| Author  : Francis Maes                   |                implementations  |
| Started : 05/01/2009 01:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CORE_IMPL_H_
# define LBCPP_CORE_IMPL_H_

# include "ContinuousFunction/FunctionStaticToDynamic.hpp"
# include "ContinuousFunction/FunctionDynamicToStatic.hpp"
# include "ContinuousFunction/ScalarFunctions.hpp"
# include "ContinuousFunction/ScalarVectorFunctions.hpp"

# include "ContinuousFunction/AbsoluteLossFunction.hpp"
# include "ContinuousFunction/SquareLossFunction.hpp"
# include "ContinuousFunction/PerceptronLossFunction.hpp"
# include "ContinuousFunction/HingeLossFunction.hpp"
# include "ContinuousFunction/LogBinomialLossFunction.hpp"
# include "ContinuousFunction/ExponentialLossFunction.hpp"
# include "ContinuousFunction/MulticlassLogBinomialLossFunction.hpp"
# include "ContinuousFunction/AllPairsLossFunction.hpp"
# include "ContinuousFunction/MostViolatedPairLossFunction.hpp"
# include "ContinuousFunction/BestAgainstAllLossFunction.hpp"

# include "ContinuousFunction/EmpiricalRiskFunction.hpp"
# include "ContinuousFunction/ExampleRiskFunction.hpp"

# include "ContinuousFunction/LinearArchitecture.hpp"
# include "ContinuousFunction/MultiLinearArchitecture.hpp"
# include "ContinuousFunction/BiasArchitecture.hpp"
# include "ContinuousFunction/TransferArchitecture.hpp"
# include "ContinuousFunction/ScalarToVectorArchitecture.hpp"

/*# include "ChooseFunction/ChooseFunctionStaticToDynamic.hpp"
# include "ChooseFunction/ChooseFunctionDynamicToStatic.hpp"
# include "ChooseFunction/CompositeValueFunctions.hpp"
# include "ChooseFunction/ClassifierValueFunctions.hpp"
# include "ChooseFunction/RegressorValueFunctions.hpp"
# include "ChooseFunction/RankerValueFunctions.hpp"*/
  
# include "Bridge/FeatureGenerator.hpp"
# include "Bridge/DoubleVector.hpp"
# include "Bridge/Choose.hpp"
# include "Bridge/Callback.hpp"
# include "Bridge/CRAlgorithmScope.hpp"
# include "Bridge/CRAlgorithm.hpp"

#endif // !LBCPP_CORE_IMPL_H_
