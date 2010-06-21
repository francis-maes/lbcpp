/*-----------------------------------------.---------------------------------.
| Filename: InferenceBaseClasses.cpp       | Inference base classes          |
| Author  : Francis Maes                   |                                 |
| Started : 27/05/2010 16:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/ParameterizedInference.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
using namespace lbcpp;

/*
** ParameterizedInference
*/
ObjectPtr ParameterizedInference::clone() const
{
  ParameterizedInferencePtr res = createAndCast<ParameterizedInference>(getClassName());
  jassert(res);
  res->parameters = parameters ? parameters->cloneAndCast<DenseVector>() : DenseVectorPtr();
  res->learner = learner ? learner->cloneAndCast<InferenceOnlineLearner>() : InferenceOnlineLearnerPtr();
  res->name = name;
  return res;
}

bool ParameterizedInference::load(InputStream& istr)
  {return Inference::load(istr) && lbcpp::read(istr, parameters);}

void ParameterizedInference::save(OutputStream& ostr) const
{
  Inference::save(ostr);
  lbcpp::write(ostr, parameters);
}

/*
** SequentialInference
*/
String SequentialInference::toString() const
{
  String res = getClassName();/* + T("(");
  size_t n = getNumSubInferences();
  for (size_t i = 0; i < n; ++i)
  {
    InferencePtr step = getSubInference(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");*/
  return res;
}

void StaticSequentialInference::getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
{
  subObjects.resize(getNumSubInferences());
  for (size_t i = 0; i < subObjects.size(); ++i)
  {
    InferencePtr subInference = getSubInference(i);
    subObjects[i].first = subInference->getName();
    subObjects[i].second = subInference;
  }
}

/*
** ParallelInference
*/
void StaticParallelInference::getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
{
  subObjects.resize(getNumSubInferences());
  for (size_t i = 0; i < subObjects.size(); ++i)
  {
    InferencePtr subInference = getSubInference(i);
    if (subInference)
    {
      subObjects[i].first = subInference->getName();
      subObjects[i].second = subInference;
    }
  }
}

SharedParallelInference::SharedParallelInference(const String& name, InferencePtr subInference)
  : StaticParallelInference(name), subInference(subInference) {}

ObjectPtr SharedParallelInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  subInference->beginRunSession();
  ObjectPtr res = ParallelInference::run(context, input, supervision, returnCode);
  subInference->endRunSession();
  return res;
}

String SharedParallelInference::toString() const
{
  jassert(subInference);
  return getClassName() + T("(") + subInference->toString() + T(")");
}

bool SharedParallelInference::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;

  subInference = createFromFileAndCast<Inference>(file.getChildFile(T("shared.inference")));
  return subInference != InferencePtr();
}

bool SharedParallelInference::saveToFile(const File& file) const
  {return saveToDirectory(file) && subInference->saveToFile(file.getChildFile(T("shared.inference")));}

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

ObjectPtr DecoratorInference::clone() const
{
  DecoratorInferencePtr res = createAndCast<DecoratorInference>(getClassName());
  res->decorated = decorated->clone().dynamicCast<Inference>();
  res->learner = learner ? learner->cloneAndCast<InferenceOnlineLearner>() : InferenceOnlineLearnerPtr();
  res->name = name;
  return res;
}

ObjectPtr DecoratorInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  if (decorated)
    return context->runInference(decorated, input, supervision, returnCode);
  else
  {
    returnCode = Inference::errorReturnCode;
    return ObjectPtr();
  }
}

void DecoratorInference::getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
{
  subObjects.resize(1);
  subObjects[0].first = decorated->getName();
  subObjects[0].second = decorated;
}
