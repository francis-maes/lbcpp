/*-----------------------------------------.---------------------------------.
| Filename: Object.h                       | A base class for serializable   |
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 17:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_OBJECT_H_
# define CRALGO_OBJECT_H_

# include "ContainerTraits.h"
# include "ReferenceCountedObject.h"

namespace cralgo
{

class ErrorHandler
{
public:
  virtual ~ErrorHandler() {}
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;
  virtual void warningMessage(const std::string& where, const std::string& what) = 0;

  static void setInstance(ErrorHandler& handler);
  static ErrorHandler& getInstance() {assert(instance); return *instance;}
  
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}
  
private:
  static ErrorHandler* instance;
};

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;

class Object : public ReferenceCountedObject
{
public:
  virtual ~Object() {}
  
  typedef Object* (*Constructor)();
    
  static void declare(const std::string& className, Constructor constructor);
  static Object* create(const std::string& className);
  
  static ObjectPtr loadFromStream(std::istream& istr);
  static ObjectPtr loadFromFile(const std::string& fileName);
  
  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromStreamCast(std::istream& istr)
    {return checkCast<T>("Object::createFromStreamCast", loadFromStream(istr));}

  template<class T>
  static ReferenceCountedObjectPtr<T> loadFromFileCast(const std::string& fileName)
    {return checkCast<T>("Object::createFromFileCast", loadFromFile(fileName));}

  std::string getClassName() const;
  virtual std::string getName() const
    {return getClassName() + "::getName() unimplemented";}
    
  virtual std::string toString() const
    {return getClassName() + "::toString() unimplemented";}
    
  bool saveToFile(const std::string& fileName) const;
  void saveToStream(std::ostream& ostr) const;

  static void error(const std::string& where, const std::string& what)
    {ErrorHandler::error(where, what);}
    
  static void warning(const std::string& where, const std::string& what)
    {ErrorHandler::warning(where, what);}
  
protected:
  template<class T>
  static ReferenceCountedObjectPtr<T> checkCast(const std::string& where, ObjectPtr object)
  {
    ReferenceCountedObjectPtr<T> res;
    if (object)
    {
      res = object.dynamicCast<T>();
      if (!res)
        error(where, "Could not cast object into '" + cralgo::toString(typeid(*res)) + "'");
    }
    return res;
  }

  virtual bool load(std::istream& istr) {return true;}
  virtual void save(std::ostream& ostr) const {}
};

template<class Type>
struct ObjectConstructor_
  {static Object* construct() {return new Type();} };
#define DECLARE_CRALGO_CLASS(Name) \
  cralgo::Object::declare(cralgo::toString(typeid(Name)), cralgo::ObjectConstructor_<Name>::construct)

template<class T>
struct ObjectPtrTraits
{
public:
  static inline std::string toString(const ReferenceCountedObjectPtr<T> value)
    {return value ? value->toString() : "null";}
  static inline void write(std::ostream& ostr, const ReferenceCountedObjectPtr<T> value)
    {assert(value); value->saveToStream(ostr);}
  static inline bool read(std::istream& istr, ReferenceCountedObjectPtr<T>& result)
    {result = Object::loadFromStreamCast<T>(istr); return result;}
};

template<>
struct Traits<ObjectPtr> : public ObjectPtrTraits<Object> {};

}; /* namespace cralgo */

#endif // !CRALGO_OBJECT_H_
