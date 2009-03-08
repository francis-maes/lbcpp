/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/Object.h>
#include <map>
#include <fstream>
using namespace cralgo;

extern void declareStandardCRAlgoClasses();
class ObjectFactory
{
public:
  ObjectFactory()
    {declareStandardCRAlgoClasses();}

  void declare(const std::string& className, Object::Constructor constructor)
  {
    if (className.empty())
      ErrorHandler::error("Object::declare", "Empty class name");
    else if (constructors.find(className) != constructors.end())
      ErrorHandler::error("Object::declare", "Class '" + className + "' is already declared.");
    else
      constructors[className] = constructor;
  }

  Object* create(const std::string& className)
  {
    if (className.empty())
    {
      ErrorHandler::error("Object::create", "Empty class name");
      return NULL;
    }
    ObjectConstructorMap::const_iterator it = constructors.find(className);
    if (it == constructors.end())
    {
      ErrorHandler::error("Object::create", "Could not find class '" + className + "'");
      return NULL;
    }
    Object* res = it->second();
    assert(res);
    return res;
  }

private:
  typedef std::map<std::string, Object::Constructor> ObjectConstructorMap;

  ObjectConstructorMap constructors;
};

static ObjectFactory objectFactory;

void Object::declare(const std::string& className, Constructor constructor)
  {objectFactory.declare(className, constructor);}

Object* Object::create(const std::string& className)
  {return objectFactory.create(className);}

ObjectPtr Object::loadFromStream(std::istream& istr)
{
  std::string className;
  if (!read(istr, className))
  {
    ErrorHandler::error("Object::create", "Could not read class name");
    return ObjectPtr();
  }
  ObjectPtr res(create(className));
  if (res && !res->load(istr))
    ErrorHandler::error("Object::create", "Could not load object of class '" + className + "'");
  return res;
}

ObjectPtr Object::loadFromFile(const std::string& fileName)
{
  std::ifstream istr(fileName.c_str(), std::ios::binary);
  if (!istr.is_open())
  {
    ErrorHandler::error("Object::createFromFile", "Could not open file '" + fileName + "'");
    return ObjectPtr();
  }
  return loadFromStream(istr);
}

std::string Object::getClassName() const
{
  return cralgo::toString(typeid(*this));
}

void Object::saveToStream(std::ostream& ostr) const
{
  write(ostr, getClassName());
  save(ostr);
}

bool Object::saveToFile(const std::string& fileName) const
{
  std::ofstream ostr(fileName.c_str());
  if (!ostr.is_open())
  {
    ErrorHandler::error("Object::saveToFile", "Could not open file '" + fileName + "'");
    return false;
  }
  saveToStream(ostr);
  ostr.flush();
  return true;
}
