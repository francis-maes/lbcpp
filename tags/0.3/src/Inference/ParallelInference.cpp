/*-----------------------------------------.---------------------------------.
| Filename: ParallelInference.cpp          | Parallel Inference              |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 22:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/ParallelInference.h>
using namespace lbcpp;

/*
** StaticParallelInference
*/
StaticParallelInference::StaticParallelInference(const String& name)
  : ParallelInference(name)
{
  setBatchLearner(staticParallelInferenceLearner());
}


/*
** SharedParallelInference
*/
SharedParallelInference::SharedParallelInference(const String& name, InferencePtr subInference)
  : StaticParallelInference(name), subInference(subInference)
{
  setBatchLearner(sharedParallelInferenceLearner());
}

Variable SharedParallelInference::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  subInference->beginRunSession();
  Variable res = ParallelInference::run(context, input, supervision, returnCode);
  subInference->endRunSession();
  return res;
}

String SharedParallelInference::toString() const
{
  jassert(subInference);
  return getClassName() + T("(") + subInference->toString() + T(")");
}

/*
** VectorParallelInference
*/
namespace lbcpp
{

  ClassPtr parallelInferenceClass()
    {static TypeCache cache(T("ParallelInference")); return cache();}

  ClassPtr staticParallelInferenceClass()
    {static TypeCache cache(T("StaticParallelInference")); return cache();}

  ClassPtr sharedParallelInferenceClass()
    {static TypeCache cache(T("SharedParallelInference")); return cache();}

  ClassPtr vectorParallelInferenceClass()
    {static TypeCache cache(T("VectorParallelInference")); return cache();}

  class SharedParallelInferenceClass : public DynamicClass
  {
  public:
    SharedParallelInferenceClass()
      : DynamicClass(T("SharedParallelInference"), parallelInferenceClass())
    {
      addVariable(inferenceClass(), T("subInference"));
    }

    LBCPP_DECLARE_VARIABLE_BEGIN(SharedParallelInference)
      LBCPP_DECLARE_VARIABLE(subInference);
    LBCPP_DECLARE_VARIABLE_END();
  };

  class VectorParallelInferenceClass : public DynamicClass
  {
  public:
    VectorParallelInferenceClass()
      : DynamicClass(T("VectorParallelInference"), staticParallelInferenceClass())
    {
      addVariable(vectorClass(inferenceClass()), T("subInferences"));
    }
    
    LBCPP_DECLARE_VARIABLE_BEGIN(VectorParallelInference)
      LBCPP_DECLARE_VARIABLE(subInferences);
    LBCPP_DECLARE_VARIABLE_END()
  };

}; /* namespace lbcpp */

void declareParallelInferenceClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(ParallelInference, Inference);
    LBCPP_DECLARE_ABSTRACT_CLASS(StaticParallelInference, ParallelInference);
      Type::declare(new SharedParallelInferenceClass());
      Type::declare(new VectorParallelInferenceClass());
}
