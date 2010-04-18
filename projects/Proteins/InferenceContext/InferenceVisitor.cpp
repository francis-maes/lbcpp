/*-----------------------------------------.---------------------------------.
| Filename: InferenceVisitor.cpp           | Inference visitor base class    |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 14:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "InferenceVisitor.h"
#include "../InferenceStep/SequentialInferenceStep.h"
#include "../InferenceStep/ParallelInferenceStep.h"
using namespace lbcpp;

void DefaultInferenceVisitor::visit(SequentialInferenceStepPtr inference)
{
  for (size_t i = 0; i < inference->getNumSubSteps(); ++i)
    inference->getSubStep(i)->accept(InferenceVisitorPtr(this));
}

void DefaultInferenceVisitor::visit(SharedParallelInferenceStepPtr inference)
  {inference->getSharedInferenceStep()->accept(InferenceVisitorPtr(this));}
