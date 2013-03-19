/*-----------------------------------------.---------------------------------.
| Filename: SimpleIntrospectionExample.h   | Illustrates Simple Introspection|
| Author  : Francis Maes                   |  operations                     |
| Started : 16/12/2010 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXAMPLES_SIMPLE_INTROSPECTION_H_
# define EXAMPLES_SIMPLE_INTROSPECTION_H_

# include <oil/Core.h>

namespace lbcpp
{

class SimpleClass : public Object
{
public:
  SimpleClass(bool b)
    : myBool(true), myInt(1664), myDouble(8.6), myString(T("yo")) {}
  SimpleClass() {}

private:
  friend class SimpleClassClass;

  bool myBool;
  int myInt;
  double myDouble;
  string myString;
  ObjectPtr myObject;
};

typedef ReferenceCountedObjectPtr<SimpleClass> SimpleClassPtr;

class SimpleIntrospectionExample : public WorkUnit
{
public:
  virtual ObjectPtr run(ExecutionContext& context)
  {
    SimpleClassPtr defaultObject = new SimpleClass();
    SimpleClassPtr someObject = new SimpleClass(true);
    context.resultCallback(T("Default Object"), defaultObject);
    context.resultCallback(T("Default Object as string"), defaultObject->toString());
    context.resultCallback(T("Some Object"), someObject);
    
    juce::File file = juce::File::createTempFile(T("object"));
    someObject->saveToFile(context, file); 

    SimpleClassPtr loadedObject = Object::createFromFile(context, file).staticCast<SimpleClass>();
    context.resultCallback(T("Loaded Object"), loadedObject);
    file.deleteFile();

    return ObjectPtr();
  }
};

}; /* namespace lbcpp */

#endif // !EXAMPLES_SIMPLE_INTROSPECTION_H_
