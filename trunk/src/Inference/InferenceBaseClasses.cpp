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
** DecoratorInference
*/
String DecoratorInference::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

bool DecoratorInference::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;
  decorated = createFromFileAndCast<Inference>(file.getChildFile(T("decorated.inference")));
  return decorated != InferencePtr();
}

bool DecoratorInference::saveToFile(const File& file) const
{
  return saveToDirectory(file) &&
    decorated->saveToFile(file.getChildFile(T("decorated.inference")));
}

ObjectPtr DecoratorInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  if (decorated)
    return decorated->run(context, input, supervision, returnCode);
  else
  {
    returnCode = Inference::errorReturnCode;
    return ObjectPtr();
  }
}

void DecoratorInference::accept(InferenceVisitorPtr visitor)
{
  if (decorated)
    decorated->accept(visitor);
}

/*
** CallbackBasedDecoratorInference
*/
bool CallbackBasedDecoratorInference::load(InputStream& istr)
  {return DecoratorInference::load(istr) && lbcpp::read(istr, callback);}

void CallbackBasedDecoratorInference::save(OutputStream& ostr) const
  {DecoratorInference::save(ostr); lbcpp::write(ostr, callback);}

ObjectPtr CallbackBasedDecoratorInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  jassert(callback);
  context->appendCallback(callback);
  ObjectPtr res = DecoratorInference::run(context, input, supervision, returnCode);
  context->removeCallback(callback);
  return res;
}

/*
** SequentialInference
*/
String SequentialInference::toString() const
{
  String res = getClassName() + T("(");
  size_t n = getNumSubSteps();
  for (size_t i = 0; i < n; ++i)
  {
    InferencePtr step = getSubStep(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");
}

void SequentialInference::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SequentialInferencePtr(this));}

ObjectPtr SequentialInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  size_t n = getNumSubSteps();
  ObjectPtr currentData = input;
  for (size_t i = 0; i < n; ++i)
  {
    InferencePtr step = getSubStep(i);
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
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference);
}
