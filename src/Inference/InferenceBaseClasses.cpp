/*-----------------------------------------.---------------------------------.
| Filename: InferenceBaseClasses.cpp       | Inference base classes          |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
using namespace lbcpp;


/*
** DecoratorInferenceStep
*/
String DecoratorInferenceStep::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

bool DecoratorInferenceStep::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;
  decorated = createFromFileAndCast<InferenceStep>(file.getChildFile(T("decorated.inference")));
  return decorated != InferenceStepPtr();
}

bool DecoratorInferenceStep::saveToFile(const File& file) const
{
  return saveToDirectory(file) &&
    decorated->saveToFile(file.getChildFile(T("decorated.inference")));
}

ObjectPtr DecoratorInferenceStep::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  if (decorated)
    return decorated->run(context, input, supervision, returnCode);
  else
  {
    returnCode = InferenceStep::errorReturnCode;
    return ObjectPtr();
  }
}

void DecoratorInferenceStep::accept(InferenceVisitorPtr visitor)
{
  if (decorated)
    decorated->accept(visitor);
}

/*
** CallbackBasedDecoratorInferenceStep
*/
bool CallbackBasedDecoratorInferenceStep::load(InputStream& istr)
  {return DecoratorInferenceStep::load(istr) && lbcpp::read(istr, callback);}

void CallbackBasedDecoratorInferenceStep::save(OutputStream& ostr) const
  {DecoratorInferenceStep::save(ostr); lbcpp::write(ostr, callback);}

ObjectPtr CallbackBasedDecoratorInferenceStep::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  jassert(callback);
  context->appendCallback(callback);
  ObjectPtr res = DecoratorInferenceStep::run(context, input, supervision, returnCode);
  context->removeCallback(callback);
  return res;
}

/*
** SequentialInferenceStep
*/
String SequentialInferenceStep::toString() const
{
  String res = getClassName() + T("(");
  size_t n = getNumSubSteps();
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");
}

void SequentialInferenceStep::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SequentialInferenceStepPtr(this));}

ObjectPtr SequentialInferenceStep::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  size_t n = getNumSubSteps();
  ObjectPtr currentData = input;
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    ObjectPtr currentSupervision = supervision ? getSubSupervision(supervision, i) : ObjectPtr();
    currentData = context->runInference(step, currentData, currentSupervision, returnCode);
    if (returnCode != finishedReturnCode)
      return ObjectPtr();
    jassert(currentData);
  }
  return currentData;
}

void declareInferenceClasses()
{
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(RegressionInferenceStep);
  LBCPP_DECLARE_CLASS(TransferRegressionInferenceStep);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInferenceStep);
}
