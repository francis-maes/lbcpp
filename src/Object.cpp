/*-----------------------------------------.---------------------------------.
| Filename: Object.cpp                     | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Object.h>
#include <lbcpp/Object/Variable.h>
#include <fstream>
using namespace lbcpp;

extern void declareLBCppCoreClasses();

String Object::getClassName() const
  {return lbcpp::toString(typeid(*this));}

ClassPtr Object::getClass() const
{
  ClassPtr res = Class::get(getClassName());
  if (!res)
    Object::error(T("Object::getClass"), T("Could not find objects class, className = ") + getClassName());
  return res;
}

ObjectPtr Object::create(const String& className)
{
  String name = className;
  if (name == T("Protein"))
    name = T("ProteinObject"); // tmp
  return Class::createInstance(name);
}

/*
** Create and load
*/
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
  ObjectPtr res = create(className);
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

Variable Object::getVariable(size_t index) const
  {jassert(false); return Variable();}

void Object::accept(ObjectVisitorPtr visitor)
{
  ClassPtr type = getClass();
  size_t n = type->getNumStaticVariables();
  for (size_t i = 0; i < n; ++i)
    visitor->visit(i, getVariable(i));
}
