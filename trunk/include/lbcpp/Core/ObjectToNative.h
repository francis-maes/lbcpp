/*-----------------------------------------.---------------------------------.
| Filename: ObjectToNative.h               | Convert Object into Native      |
| Author  : Francis Maes                   |  C++ variables                  |
| Started : 12/11/2012 16:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_OBJECT_TO_NATIVE_H_
# define LBCPP_CORE_OBJECT_TO_NATIVE_H_

# include "Object.h"

namespace lbcpp
{

/*
** Atomic Types
*/
inline void objectToNative(ExecutionContext& context, bool& dest, const ObjectPtr& source)
  {dest = Boolean::get(source);}

inline void objectToNative(ExecutionContext& context, int& dest, const ObjectPtr& source)
  {dest = (int)Integer::get(source);}

inline void objectToNative(ExecutionContext& context, juce::int64& dest, const ObjectPtr& source)
  {dest = Integer::get(source);}

inline void objectToNative(ExecutionContext& context, size_t& dest, const ObjectPtr& source)
  {dest = (size_t)Integer::get(source);}
 
inline void objectToNative(ExecutionContext& context, unsigned char& dest, const ObjectPtr& source)
  {dest = (unsigned char)Integer::get(source);}

inline void objectToNative(ExecutionContext& context, double& dest, const ObjectPtr& source)
  {dest = Double::get(source);}

inline void objectToNative(ExecutionContext& context, string& dest, const ObjectPtr& source)
  {dest = String::get(source);}

inline void objectToNative(ExecutionContext& context, juce::File& dest, const ObjectPtr& source)
  {dest = File::get(source);}

inline void objectToNative(ExecutionContext& context, ObjectPtr& dest, const ObjectPtr& source)
  {dest = source;}

template<class TT>
inline void objectToNative(ExecutionContext& context, ReferenceCountedObjectPtr<TT>& dest, const ObjectPtr& source)
  {dest = source.staticCast<TT>();}

template<class TT>
inline void objectToNative(ExecutionContext& context, TT*& dest, const ObjectPtr& source)
  {dest = source.staticCast<TT>().get();}

/*
** Pair
*/
template<class T1, class T2>
inline void objectToNative(ExecutionContext& context, std::pair<T1, T2>& dest, const ObjectPtr& source)
{
  const PairPtr& sourcePair = source.staticCast<Pair>();
  if (sourcePair)
  {
    lbcpp::objectToNative(context, dest.first, sourcePair->getFirst());
    lbcpp::objectToNative(context, dest.second, sourcePair->getSecond());
  }
  else
  {
    dest.first = T1();
    dest.second = T2();
  }
}

/*
** Vector
*/
template<class TT>
inline void objectToNative(ExecutionContext& context, std::vector<TT>& dest, const ObjectPtr& source)
{
  const VectorPtr& sourceVector = source.staticCast<Vector>();
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
      lbcpp::objectToNative(context, dest[i], sourceVector->getElement(i));
  }
  else
    dest.clear();
}
// I duplicated function for bool case because xcode convert bool to std::_Bit_reference Grrrr
inline void objectToNative(ExecutionContext& context, std::vector<bool>& dest, const ObjectPtr& source)
{
  const VectorPtr& sourceVector = source.staticCast<Vector>();
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
    {
      ObjectPtr elt = sourceVector->getElement(i);
      dest[i] = elt && Boolean::get(elt);
    }
  }
  else
    dest.clear();
}

template<class TT>
inline void objectToNative(ExecutionContext& context, std::set<TT>& dest, const ObjectPtr& source)
{
  dest.clear();
  const VectorPtr& sourceContainer = source.staticCast<Vector>(context);
  if (sourceContainer)
  {
    size_t n = sourceContainer->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      TT value;
      lbcpp::objectToNative(context, value, sourceContainer->getElement(i));
      dest.insert(value);
    }
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_OBJECT_TO_NATIVE_H_
