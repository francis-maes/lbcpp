/*-----------------------------------------.---------------------------------.
| Filename: Ind..Mul..Distri..Builder.cpp  | DistributionBuilder associated  |
| Author  : Arnaud Schoofs                 | with IndependentMultiVariate    |
| Started : 08/03/2011                     | Distribution                    |
`------------------------------------------/ (implementation file)           |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "IndependentMultiVariateDistributionBuilder.h"

using namespace lbcpp;

IndependentMultiVariateDistributionBuilder::IndependentMultiVariateDistributionBuilder(ClassPtr elementsType) : 
DistributionBuilder(independentMultiVariateDistributionBuilderClass(elementsType)), distributionsBuilders(elementsType->getNumMemberVariables()) {}

