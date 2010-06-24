/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Object.h>
#include <map>
#include <fstream>
using namespace lbcpp;

extern void declareLBCppCoreClasses();

class ObjectFactory
{
public:
  void declare(const String& className, Object::Constructor constructor)
  {
    ScopedLock _(lock);

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

  bool doClassNameExists(const String& className) const
  {
    ScopedLock _(lock);
    return constructors.find(className) != constructors.end();
  }

  Object* create(const String& className)
  {
    ScopedLock _(lock);

    // check that lbc++ core classes have been declared
    if (constructors.find(T("SparseVector")) == constructors.end())
      declareLBCppCoreClasses();
      
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
  CriticalSection lock;
};

inline ObjectFactory& getObjectFactoryInstance()
{
  static ObjectFactory instance;
  return instance;
}

String Object::getClassName() const
  {return lbcpp::toString(typeid(*this));}

void Object::declare(const String& className, Constructor constructor)
  {getObjectFactoryInstance().declare(className, constructor);}

bool Object::doClassNameExists(const String& className)
  {return getObjectFactoryInstance().doClassNameExists(className);}

/*
** Create and load
*/
Object* Object::create(const String& className)
  {return getObjectFactoryInstance().create(className);}

ObjectPtr Object::createFromStream(InputStream& istr, bool doLoading)
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
  if (res && doLoading && !res->load(istr))
    error(T("Object::create"), T("Could not load object of class ") + className);
  return res;
}

ObjectPtr Object::createFromFile(const File& file)
{
  if (!file.exists())
  {
    error(T("Object::createFromFile"), T("File ") + file.getFullPathName() + T(" does not exists"));
    return ObjectPtr();
  }
  
  File f = file.isDirectory() ? file.getChildFile(T(".classFile")) : file;
  InputStream* inputStream = f.createInputStream();
  if (!inputStream)
  {
    error(T("Object::createFromFile"), T("Could not open file ") + f.getFullPathName());
    return ObjectPtr();
  }
  ObjectPtr res = createFromStream(*inputStream, false);
  if (!res)
  {
    delete inputStream;
    return ObjectPtr();
  }
  if (file.isDirectory())
  {
    // loading of a directory: once we have the classname, we close the input stream and let the Object do want it wants
    // most of the time, this may include re-opening the file to read data beyond the classname
    delete inputStream;
    res->loadFromFile(file);
  }
  else
  {
    // loading of a file: now that we have the classname, we just continue reading from the input stream
    if (!res->load(*inputStream))
      error(T("Object::create"), T("Could not load object from file ") + file.getFullPathName());
    delete inputStream;
  }
  return res;
}

/*
** Load
*/
bool Object::loadFromFile(const File& file)
{
  InputStream* inputStream = file.createInputStream();
  if (!inputStream)
  {
    error(T("Object::loadFromFile"), T("Could not open file ") + file.getFullPathName());
    return false;
  }
  String className;
  if (!read(*inputStream, className))
  {
    error(T("Object::loadFromFile"), T("Could not read class name"));
    delete inputStream;
    return false;
  }
  if (className != getClassName())
  {
    error(T("Object::loadFromFile"), T("Class name mismatch"));
    delete inputStream;
    return false;
  }
  if (!load(*inputStream))
  {
    error(T("Object::loadFromFile"), T("Could not load object of class ") + className);
    delete inputStream;
    return false;
  }
  delete inputStream;
  return true;
}

bool Object::loadFromDirectory(const File& directory)
{
  if (!directory.exists() || !directory.isDirectory())
  {
    error(T("Object::loadFromDirectory"), directory.getFullPathName() + T(" is not a directory"));
    return false;
  }
  return Object::loadFromFile(directory.getChildFile(T(".classFile")));
}

/*
** Save
*/
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

bool Object::saveToDirectory(const File& directory) const
{
  if (directory.existsAsFile())
    directory.deleteFile();
  if (!directory.exists() && !directory.createDirectory())
  {
    error(T("Object::saveToFile"), T("Could not create directory ") + directory.getFullPathName());
    return false;
  }
  return Object::saveToFile(directory.getChildFile(T(".classFile")));
}
