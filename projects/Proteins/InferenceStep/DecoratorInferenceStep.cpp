/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceStep.cpp     | Decorator inference steps       |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 11:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "DecoratorInferenceStep.h"
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
