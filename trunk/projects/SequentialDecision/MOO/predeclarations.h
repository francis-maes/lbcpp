/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Multi Objective Optimization    |
| Author  : Francis Maes                   | predeclarations                 |
| Started : 14/09/2012 12:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PREDECLARATIONS_H_
# define LBCPP_MOO_PREDECLARATIONS_H_

# include <lbcpp/Core/Object.h>

namespace lbcpp
{

class MOODomain;
typedef ReferenceCountedObjectPtr<MOODomain> MOODomainPtr;

class ContinuousDomain;
typedef ReferenceCountedObjectPtr<ContinuousDomain> ContinuousDomainPtr;

class MOOFitnessLimits;
typedef ReferenceCountedObjectPtr<MOOFitnessLimits> MOOFitnessLimitsPtr;

class MOOFitness;
typedef ReferenceCountedObjectPtr<MOOFitness> MOOFitnessPtr;

class MOOSolution;
typedef ReferenceCountedObjectPtr<MOOSolution> MOOSolutionPtr;

class MOOSolutionComparator;
typedef ReferenceCountedObjectPtr<MOOSolutionComparator> MOOSolutionComparatorPtr;

class MOOProblem;
typedef ReferenceCountedObjectPtr<MOOProblem> MOOProblemPtr;

class MOOSolutionSet;
typedef ReferenceCountedObjectPtr<MOOSolutionSet> MOOSolutionSetPtr;

class MOOParetoFront;
typedef ReferenceCountedObjectPtr<MOOParetoFront> MOOParetoFrontPtr;

class MOOOptimizer;
typedef ReferenceCountedObjectPtr<MOOOptimizer> MOOOptimizerPtr;

class IterativeOptimizer;
typedef ReferenceCountedObjectPtr<IterativeOptimizer> IterativeOptimizerPtr;

class MOOSampler;
typedef ReferenceCountedObjectPtr<MOOSampler> MOOSamplerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PREDECLARATIONS_H_
