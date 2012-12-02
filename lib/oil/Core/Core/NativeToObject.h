/*-----------------------------------------.---------------------------------.
| Filename: NativeToObject.h               | Convert Native C++ variables    |
| Author  : Francis Maes                   |  to Objects                     |
| Started : 12/11/2012 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_NATIVE_TO_OBJECT_H_
# define LBCPP_CORE_NATIVE_TO_OBJECT_H_

# include "Object.h"

namespace lbcpp
{
  
/*
** Atomic Types
*/
inline ObjectPtr nativeToObject(const bool& source, const ClassPtr& expectedType)
  {return new Boolean(expectedType, source);}

inline ObjectPtr nativeToObject(const unsigned char& source, const ClassPtr& expectedType)
  {return Integer::create(expectedType, source);}
inline ObjectPtr nativeToObject(const size_t& source, const ClassPtr& expectedType)
  {return Integer::create(expectedType, source);}
inline ObjectPtr nativeToObject(const int& source, const ClassPtr& expectedType)
  {return Integer::create(expectedType, source);}
inline ObjectPtr nativeToObject(const juce::int64& source, const ClassPtr& expectedType)
  {return Integer::create(expectedType, source);}

inline ObjectPtr nativeToObject(const float& source, const ClassPtr& expectedType)
  {return Double::create(expectedType, source);}

inline ObjectPtr nativeToObject(const double& source, const ClassPtr& expectedType)
  {return Double::create(expectedType, source);}

inline ObjectPtr nativeToObject(const string& source, const ClassPtr& expectedType)
  {return new String(expectedType, source);}

inline ObjectPtr nativeToObject(const juce::File& source, const ClassPtr& expectedType)
  {return new File(expectedType, source);}

template<class TT>
inline ObjectPtr nativeToObject(const ReferenceCountedObjectPtr<TT>& source, const ClassPtr& expectedType)
  {return source;}

template<class TT>
inline ObjectPtr nativeToObject(const TT* source, const ClassPtr& expectedType)
  {return ObjectPtr(source);}

template<class TT>
inline ObjectPtr nativeToObject(const TT& source, const ClassPtr& expectedType)
  {return ObjectPtr(&source);}

/*
** Pair
*/
template<class T1, class T2>
inline ObjectPtr nativeToObject(const std::pair<T1, T2>& source, const ClassPtr& expectedType)
{
  jassert(expectedType->getNumTemplateArguments() == 2);
  return new Pair(expectedType, nativeToObject(source.first, expectedType->getTemplateArgument(0)),
                                nativeToObject(source.second, expectedType->getTemplateArgument(1)));
}

/*
** Vector
*/
template<class TT>
inline ObjectPtr nativeToObject(const std::vector<TT>& source, const ClassPtr& expectedType)
{
  VectorPtr res = Vector::create(expectedType.staticCast<Class>()).staticCast<Vector>();

  res->resize(source.size());
  ClassPtr elementsType = res->getElementsType();
  for (size_t i = 0; i < source.size(); ++i)
  {
    ObjectPtr variable = nativeToObject(source[i], elementsType);
    if (variable)
      res->setElement(i, variable);
  }
  return res;
}

template<class TT>
inline ObjectPtr nativeToObject(const std::set<TT>& source, const ClassPtr& expectedType)
{
  VectorPtr res = Vector::create(expectedType.staticCast<Class>()).staticCast<Vector>();

  res->resize(source.size());
  ClassPtr elementsType = res->getElementsType();
  size_t i = 0;
  typedef typename std::set<TT>::const_iterator iterator;
  for (iterator it = source.begin(); it != source.end(); ++it, ++i)
  {
    ObjectPtr variable = nativeToObject(*it, elementsType);
    if (variable)
      res->setElement(i, variable);
  }
  return res;
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_NATIVE_TO_OBJECT_H_
