/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
using namespace lbcpp;

void Inference::clone(ObjectPtr target) const
{
  NameableObject::clone(target);
  if (onlineLearner)
    InferencePtr(target)->onlineLearner = onlineLearner->cloneAndCast<InferenceOnlineLearner>();
}


/*
** Numerical
*/
#include "NumericalInference/LinearInference.h"
#include "NumericalInference/TransferFunctionDecoratorInference.h"
#include "NumericalInference/BinaryClassificationInference.h"
#include "NumericalInference/RegressionInference.h"

InferencePtr lbcpp::linearScalarInference(const String& name)
  {return new LinearInference(name);}

InferencePtr lbcpp::transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction)
  {return new TransferFunctionDecoratorInference(name, decoratedInference, transferFunction);}

InferencePtr lbcpp::binaryLinearSVMInference(InferencePtr scoreInference)
  {return new BinaryLinearSVMInference(scoreInference);}

InferencePtr lbcpp::binaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new BinaryLinearSVMInference(learner, name);}

InferencePtr lbcpp::binaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new BinaryLogisticRegressionInference(learner, name);}

InferencePtr lbcpp::squareRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new SquareRegressionInference(learner, name);}

InferencePtr lbcpp::absoluteRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new AbsoluteRegressionInference(learner, name);}

InferencePtr lbcpp::dihedralAngleRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new AngleRegressionInference(learner, name);}

/*
** Decision Tree
*/
#include "DecisionTreeInference/ExtraTreeInference.h"
#include "DecisionTreeInference/ExtraTreeInferenceLearner.h"

inline InferencePtr extraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  {return new SingleExtraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting);}

InferencePtr lbcpp::regressionExtraTreeInference(const String& name, TypePtr inputType, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new RegressionBinaryDecisionTreeInference(name, inputType);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

InferencePtr lbcpp::binaryClassificationExtraTreeInference(const String& name, TypePtr inputType, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new BinaryClassificationBinaryDecisionTreeInference(name, inputType);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

InferencePtr lbcpp::classificationExtraTreeInference(const String& name, TypePtr inputType, EnumerationPtr classes, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new ClassificationBinaryDecisionTreeInference(name, inputType, classes);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

/*
** Reduction
*/
#include "ReductionInference/OneAgainstAllClassificationInference.h"
#include "ReductionInference/ParallelVoteInference.h"
#include "ReductionInference/SharedParallelVectorInference.h"

InferencePtr lbcpp::oneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
  {return new OneAgainstAllClassificationInference(name, classes, binaryClassifierModel);}

InferencePtr lbcpp::parallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voteLearner)
  {return new ParallelVoteInference(name, numVoters, voteInferenceModel, voteLearner);}

// sizeFunction: input -> size
// perception: (input, position) pair -> object
InferencePtr lbcpp::sharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, PerceptionPtr perception, InferencePtr elementInference)
  {return new SharedParallelVectorInference(name, sizeFunction, perception, elementInference);}

/*
** Meta Inference
*/
#include "MetaInference/DummyInferenceLearner.h"
#include "MetaInference/StaticSequentialInferenceLearner.h"
#include "MetaInference/StaticParallelInferenceLearner.h"
#include "MetaInference/SharedParallelInferenceLearner.h"
#include "MetaInference/DecoratorInferenceLearner.h"
#include "MetaInference/OnlineToBatchInferenceLearner.h"

InferencePtr lbcpp::dummyInferenceLearner()
  {return new DummyInferenceLearner();}

InferencePtr lbcpp::staticSequentialInferenceLearner()
  {return new StaticSequentialInferenceLearner();}

InferencePtr lbcpp::staticParallelInferenceLearner()
  {return new StaticParallelInferenceLearner();}

InferencePtr lbcpp::sharedParallelInferenceLearner(bool filterUnsupervisedExamples)
  {return new SharedParallelInferenceLearner(filterUnsupervisedExamples);}

InferencePtr lbcpp::parallelVoteInferenceLearner()
  {return new ParallelVoteInferenceLearner();}

InferencePtr lbcpp::decoratorInferenceLearner()
  {return new DecoratorInferenceLearner();}

InferencePtr lbcpp::postProcessInferenceLearner()
  {return new PostProcessInferenceLearner();}

InferencePtr lbcpp::onlineToBatchInferenceLearner()
  {return new OnlineToBatchInferenceLearner();}

#include "MetaInference/CallbackBasedDecoratorInference.h"
#include "MetaInference/RunOnSupervisedExamplesInference.h"

InferencePtr lbcpp::runOnSupervisedExamplesInference(InferencePtr inference)
  {return new RunOnSupervisedExamplesInference(inference);}

InferencePtr lbcpp::callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback)
  {return new CallbackBasedDecoratorInference(name, decoratedInference, callback);}

//
extern void declareInferenceClasses(); // generated

extern void declareDecoratorInferenceClasses();

void declareInferenceLibrary()
{
  declareInferenceClasses(); // generated

  /*
  ** Base classes
  */
  declareDecoratorInferenceClasses();
    
  
  /*
  ** Reduction
  */
  Type::declare(new OneAgainstAllClassificationInferenceClass());
  LBCPP_DECLARE_CLASS(ParallelVoteInference, VectorParallelInference);
  Type::declare(new SharedParallelVectorInferenceClass());

  /*
  ** Numerical
  */
  Type::declare(new NumericalInferenceClass());
    LBCPP_DECLARE_CLASS(LinearInference, NumericalInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(BinaryClassificationInference, StaticDecoratorInference);
      LBCPP_DECLARE_CLASS(BinaryLinearSVMInference, BinaryClassificationInference);
      LBCPP_DECLARE_CLASS(BinaryLogisticRegressionInference, BinaryClassificationInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(RegressionInference, StaticDecoratorInference);
      LBCPP_DECLARE_CLASS(SquareRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AbsoluteRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AngleRegressionInference, RegressionInference);

    LBCPP_DECLARE_CLASS(TransferFunctionDecoratorInference, StaticDecoratorInference);

  /*
  ** Decision Tree
  */
  LBCPP_DECLARE_CLASS(BinaryDecisionTree, Object);
  LBCPP_DECLARE_CLASS(BinaryDecisionTreeInference, Inference);
    LBCPP_DECLARE_CLASS(RegressionBinaryDecisionTreeInference, BinaryDecisionTreeInference);
    LBCPP_DECLARE_CLASS(BinaryClassificationBinaryDecisionTreeInference, BinaryDecisionTreeInference);
    LBCPP_DECLARE_CLASS(ClassificationBinaryDecisionTreeInference, BinaryDecisionTreeInference);
  
  /*
  ** Meta
  */
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference, StaticDecoratorInference);  
  LBCPP_DECLARE_CLASS(OnlineToBatchInferenceLearner, Inference);
  LBCPP_DECLARE_CLASS(StaticSequentialInferenceLearner, Inference);
  LBCPP_DECLARE_CLASS(StaticParallelInferenceLearner, ParallelInference);
    LBCPP_DECLARE_CLASS(ParallelVoteInferenceLearner, StaticParallelInferenceLearner);
  LBCPP_DECLARE_CLASS(SharedParallelInferenceLearner, DecoratorInference);
  LBCPP_DECLARE_CLASS(DecoratorInferenceLearner, DecoratorInference);
    LBCPP_DECLARE_CLASS(PostProcessInferenceLearner, DecoratorInferenceLearner);
  
  LBCPP_DECLARE_CLASS(RunOnSupervisedExamplesInference, ParallelInference);
  LBCPP_DECLARE_ABSTRACT_CLASS(RunSequentialInferenceStepOnExamples, ParallelInference);
}
