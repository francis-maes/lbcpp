/*
 *  IndependentMultiVariateDistributionBuilder.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 8/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "precompiled.h"
#include "IndependentMultiVariateDistributionBuilder.h"

using namespace lbcpp;

IndependentMultiVariateDistributionBuilder::IndependentMultiVariateDistributionBuilder(ClassPtr elementsType) : 
DistributionBuilder(independentMultiVariateDistributionBuilderClass(elementsType)), distributionsBuilders(elementsType->getNumMemberVariables()) {}

