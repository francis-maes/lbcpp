/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object.h>
#include <map>
#include <fstream>
using namespace lbcpp;

extern void declareLBCppCoreClasses();

class ObjectFactory
{
public:
  ObjectFactory()
    {declareLBCppCoreClasses();}

  void declare(const String& className, Object::Constructor constructor)
  {
    if (className.isEmpty())
      Object::error(T("Object::declare"), T("Empty class name"));
    else if (constructors.find(className) != constructors.end())
      Object::error(T("Object::declare"), T("Class '") + className + T("' is already declared."));
    else
    {
      //std::cout << "Object::declare " << className << std::endl;
      constructors[className] = constructor;
    }
  }

  Object* create(const String& className)
  {
    if (className.isEmpty())
    {
      Object::error(T("Object::create"), T("Empty class name"));
      return NULL;
    }
    ObjectConstructorMap::const_iterator it = constructors.find(className);
    if (it == constructors.end())
    {
      Object::error(T("Object::create"), T("Could not find class '") + className + T("'"));
      return NULL;
    }
    Object* res = it->second();
    jassert(res);
    return res;
  }

private:
  typedef std::map<String, Object::Constructor> ObjectConstructorMap;

  ObjectConstructorMap constructors;
};

inline ObjectFactory& getObjectFactoryInstance()
{
  static ObjectFactory instance;
  return instance;
}

void Object::declare(const String& className, Constructor constructor)
  {getObjectFactoryInstance().declare(className, constructor);}

Object* Object::create(const String& className)
  {return getObjectFactoryInstance().create(className);}

ObjectPtr Object::loadFromStream(InputStream& istr)
{
  String className;
  if (!read(istr, className))
  {
    error(T("Object::create"), T("Could not read class name"));
    return ObjectPtr();
  }
  if (className == T("__null__"))
    return ObjectPtr();
  ObjectPtr res(create(className));
  if (res && !res->load(istr))
    error(T("Object::create"), T("Could not load object of class ") + className);
  return res;
}

ObjectPtr Object::loadFromFile(const File& file)
{
  InputStream* inputStream = file.createInputStream();
  if (!inputStream)
  {
    error(T("Object::createFromFile"), T("Could not open file ") + file.getFullPathName());
    return ObjectPtr();
  }
  ObjectPtr res = loadFromStream(*inputStream);
  delete inputStream;
  return res;
}

String Object::getClassName() const
{
  return lbcpp::toString(typeid(*this));
}

void Object::saveToStream(OutputStream& ostr) const
{
  ostr.writeString(getClassName());
  save(ostr);
}

bool Object::saveToFile(const File& file) const
{
  if (file.existsAsFile())
    file.deleteFile();
  OutputStream* outputStream = file.createOutputStream();
  if (!outputStream)
  {
    error(T("Object::saveToFile"), T("Could not open file ") + file.getFullPathName());
    return false;
  }
  saveToStream(*outputStream);
  outputStream->flush();
  delete outputStream;
  return true;
}
