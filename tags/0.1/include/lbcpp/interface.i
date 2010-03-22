/*-----------------------------.---------------------------------------------.
| Filename: interface.i        | SWIG Interface                              |
| Author  : Guillaume Calas    |                                             |
| Started : 10/06/2009 11:00   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

%module lbcpp
%{
#include "lbcpp.h"
using namespace lbcpp;
%}

%include "lbcpp.h"

%template(ObjectPtr) ReferenceCountedObjectPtr<Object>;

%include "CRAlgorithm.h"
%include "CRAlgorithmLearner.h"
%include "CRAlgorithmScope.h"
%include "Callback.h"
%include "Choose.h"
%include "ChooseFunction.h"
%include "ContainerTraits.h"
%include "ContinuousFunction.h"
%include "DenseVector.h"
%include "EditableFeatureGenerator.h"
%include "FeatureDictionary.h"
%include "FeatureGenerator.h"
%include "GradientBasedLearner.h"
%include "GradientBasedLearningMachine.h"
%include "IterationFunction.h"
%include "LearningExample.h"
%include "LearningMachine.h"
%include "Object.h"
%include "ObjectContainer.h"
%include "ObjectGraph.h"
%include "ObjectPredeclarations.h"
%include "ObjectStream.h"
%include "Optimizer.h"
%include "Policy.h"
%include "Random.h"
%include "RandomVariable.h"
%include "ReferenceCountedObject.h"
%include "SparseVector.h"
%include "Traits.h"
%include "Utilities.h"
%include "Variable.h"