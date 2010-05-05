/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: ContainerTraits.h              | C++ Containers Type Traits      |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 21:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTAINER_TRAITS_H_
# define LBCPP_CONTAINER_TRAITS_H_

# include "Traits.h"
# include "../RandomGenerator.h"
# include <vector>
# include <set>
# include <map>
# include <limits>

namespace lbcpp
{
/*
Traits:
  static String toString(const T& container)
  static void write(OutputStream& ostr, const T& container)
  static bool read(InputStream& istr, T& container)

ContainerTraits:
  Traits

  typedef ... ValueType;
  typedef ... ConstIterator;

  static size_t size(const T& container)
  static ConstIterator begin(const T& container)
  static ConstIterator end(const T& container)
  static const ValueType& value(const ConstIterator& iterator)

  static ValueType& sampleRandom(const T& s)
  template<class CRAlgorithmType, class ScoringFunction>
  static inline ConstIterator sampleBests(const T& container, const ScoringFunction& scoringFunction)
*/

/*!
** @struct SampleRandomImplementation
** @brief #FIXME
**
*/
template<class IteratorType>
struct SampleRandomImplementation
{
  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  template<class ContainerType>
  static inline const typename Traits<ContainerType>::ValueType& sampleRandom(const ContainerType& container)
  {
    typedef Traits<ContainerType> ContainerTraits;

    // not tested
    jassert(ContainerTraits::size(container));
    int r = RandomGenerator::getInstance().sampleSize(ContainerTraits::size(container));
    int i = 0;
    typename ContainerTraits::ConstIterator it;
    for (it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it, ++i)
      if (i == r)
        return ContainerTraits::value(it);
    jassert(false);
    return *(const typename Traits<ContainerType>::ValueType* )0;
  }
};

template<>
struct SampleRandomImplementation<size_t>
{
  template<class ContainerType>
  static inline const size_t& sampleRandom(const ContainerType& container)
  {
    static size_t result;
    typedef Traits<ContainerType> ContainerTraits;
    jassert(ContainerTraits::size(container));

    // not tested
    size_t b = ContainerTraits::begin(container);
    size_t e = ContainerTraits::end(container);
    jassert(e > b);
    result = RandomGenerator::getInstance().sampleSize(b, e);
    return result;
  }
};

// ContainerTraits: size(), begin(), end(), value()
/*!
** @struct DefaultContainerTraits
** @brief #FIXME
**
*/
template<class ContainerTraits, class ContainerType, class ValType, class IteratorType>
struct DefaultContainerTraits
{
  typedef ContainerType Type;
  typedef ValType ValueType;
  typedef IteratorType ConstIterator;

  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static inline String toString(const ContainerType& container)
  {
    String res;
    for (ConstIterator it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it)
    {
      if (res.length() > 0)
        res += T(", ");
      res += lbcpp::toString(ContainerTraits::value(it));
    }
    return res;
  }

  /*!
  **
  **
  ** @param ostr
  ** @param container
  */
  static inline void write(OutputStream& ostr, const ContainerType& container)
  {
    Traits<size_t>::write(ostr, ContainerTraits::size(container));
    for (ConstIterator it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it)
      lbcpp::write(ostr, ContainerTraits::value(it));
  }

  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static inline const ValueType& sampleRandom(const ContainerType& container)
    {return SampleRandomImplementation<ConstIterator>::sampleRandom(container);}

  /*!
  **
  **
  ** @param container
  ** @param scoringFunction
  **
  ** @return
  */
  template<class ScoringFunction>
  static inline ConstIterator sampleBests(const ContainerType& container, const ScoringFunction& scoringFunction)
  {
    std::vector<ConstIterator> bests;
    double bestsScore = -std::numeric_limits<double>::max();
    for (ConstIterator it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it)
    {
      double score = scoringFunction.compute(ContainerTraits::value(it));
      jassert(isNumberValid(score));
      if (score > bestsScore)
      {
        bests.clear();
        bests.push_back(it);
        bestsScore = score;
      }
      else if (score == bestsScore)
        bests.push_back(it);
    }
    if (bests.size() == 0)
    {
      std::cerr << "Could not find a best choice." << std::endl;
      std::cerr << "Scores: ";
      for (ConstIterator it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it)
        std::cerr << scoringFunction.compute(ContainerTraits::value(it)) << ", ";
      std::cerr << std::endl;
      jassert(false);
    }

//    std::cout << "Num bests: " << bests.size();
//    ConstIterator it = Traits< std::vector<ConstIterator> >::sampleRandom(bests);
//    std::cout << " =sample> " << lbcpp::toString(ContainerTraits::value(it)) << std::endl;
    return Traits< std::vector<ConstIterator> >::sampleRandom(bests);
  }
};

template<class ContainerType>
struct BuiltinContainerTraits : public DefaultContainerTraits<
      BuiltinContainerTraits<ContainerType>,
      ContainerType,
      typename ContainerType::value_type,
      typename ContainerType::const_iterator >
{
  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static inline size_t size(const ContainerType& container)
    {return container.size();}

  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static typename ContainerType::const_iterator begin(const ContainerType& container)
    {return container.begin();}

  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static typename ContainerType::const_iterator end(const ContainerType& container)
    {return container.end();}

  /*!
  **
  **
  ** @param iterator
  **
  ** @return
  */
  static typename ContainerType::value_type value(const typename ContainerType::const_iterator& iterator)
    {return *iterator;}

  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static inline const typename ContainerType::value_type& sampleRandom(const ContainerType& container)
  {
    // not tested
    jassert(container.size());
    int r = RandomGenerator::getInstance().sampleSize(container.size());
    int i = 0;
    //if (r < (int)container.size() / 2)
    {
      typename ContainerType::const_iterator it = container.begin();
      for (; it != container.end(); ++it, ++i)
        if (i == r)
          return *it;
    }
  /*  else
    {
      r = container.size() - 1 - r;
      typename ContainerType::const_reverse_iterator it;
      for (it = container.rbegin(); it != container.rend(); ++it, ++i)
        if (i == r)
          return *it;
    }*/
    jassert(false);
    return *(typename ContainerType::value_type* )0;
  }
};

/*
** Vectors
*/
template<class T>
struct Traits< std::vector<T> > : public BuiltinContainerTraits< std::vector<T> >
{
  static inline String toString(const std::vector<T>& vector)
    {return T("[") + BuiltinContainerTraits< std::vector<T> >::toString(vector) + T("]");}

  static inline bool read(InputStream& istr, std::vector<T>& res)
  {
    size_t size;
    if (!Traits<size_t>::read(istr, size))
      return false;
    res.resize(size);
    for (size_t i = 0; i < size; ++i)
      if (!Traits<T>::read(istr, res[i]))
        return false;
    return true;
  }

  static const T& sampleRandom(const std::vector<T>& vector)
  {
    jassert(vector.size());
    return vector[RandomGenerator::getInstance().sampleSize(vector.size())];
  }
};

template<>
struct Traits< std::vector<bool> > : public BuiltinContainerTraits< std::vector<bool> >
{
  static inline String toString(const std::vector<bool>& vector)
  {
    String res = T("[");
    for (size_t i = 0; i < vector.size(); ++i)
      res += vector[i] ? T("1") : T("0");
    res += T("]");
    return res;
  }

  static inline bool read(InputStream& istr, std::vector<bool>& res)
  {
    size_t size;
    if (!Traits<size_t>::read(istr, size))
      return false;
    res.resize(size);
    for (size_t i = 0; i < size; ++i)
    {
      bool value;
      if (!Traits<bool>::read(istr, value))
        return false;
      res[i] = value;
    }
    return true;
  }

  static bool sampleRandom(const std::vector<bool>& vector)
  {
    jassert(vector.size());
    return vector[RandomGenerator::getInstance().sampleSize(vector.size())];
  }
};

/*
** Set
*/
template<class T>
struct Traits< std::set<T> > : public BuiltinContainerTraits< std::set<T> >
{
  static inline String toString(const std::set<T>& container)
    {return T("{") + BuiltinContainerTraits< std::set<T> >::toString(container) + T("}");}
};

/*
** Map
*/
template<class KeyType, class MapValueType>
struct Traits< std::map<KeyType, MapValueType> >
  : public BuiltinContainerTraits< std::map<KeyType, MapValueType> >
{
  typedef std::map<KeyType, MapValueType> ContainerType;
  typedef typename ContainerType::const_iterator ConstIterator;

  static inline void write(OutputStream& ostr, const ContainerType& container)
  {
    size_t size = container.size();
    lbcpp::write(ostr, size);
    for (ConstIterator it = container.begin(); it != container.end(); ++it)
    {
      lbcpp::write(ostr, it->first);
      lbcpp::write(ostr, it->second);
    }
  }

  static inline bool read(InputStream& istr, ContainerType& res)
  {
    size_t count;
    if (!lbcpp::read(istr, count))
      return false;
    for (size_t i = 0; i < count; ++i)
    {
      KeyType key;
      MapValueType value;
      if (!lbcpp::read(istr, key) || !lbcpp::read(istr, value))
        return false;
      res[key] = value;
    }
    return true;
  }
};

template<class KeyType, class MapValueType>
struct Traits< std::multimap<KeyType, MapValueType> >
  : public BuiltinContainerTraits< std::multimap<KeyType, MapValueType> >
{
};


/*
** Pairs
*/
template<class T1, class T2>
struct Traits< std::pair<T1, T2> >
{
  static inline String toString(const std::pair<T1, T2>& value)
    {return T("(") + Traits<T1>::toString(value.first) + T(", ") + Traits<T2>::toString(value.second) + T(")");}
    
  static inline void write(OutputStream& ostr, const std::pair<T1, T2>& value)
  {
    Traits<T1>::write(ostr, value.first);
    Traits<T2>::write(ostr, value.second);
  }
  
  static inline bool read(InputStream& istr, std::pair<T1, T2>& result)
  {
    return Traits<T1>::read(istr, result.first) &&
      Traits<T2>::read(istr, result.second);
  }
};

/*
** Ranges
*/
/*!
** @struct SizeRange
** @brief #FIXME
**
*/
struct SizeRange
{
  /*!
  **
  **
  ** @param begin
  ** @param end
  **
  ** @return
  */
  SizeRange(size_t begin, size_t end)
    : begin(begin), end(end) {}

  /*!
  **
  **
  ** @param end
  **
  ** @return
  */
  SizeRange(size_t end = 0)
    : begin(0), end(end) {}

  size_t begin;                 /*!< */
  size_t end;                   /*!< */
};

template<>
struct Traits< SizeRange >
  : public DefaultContainerTraits< Traits<SizeRange>, SizeRange, size_t, size_t >
{
  static size_t size(const SizeRange& range)
    {return range.end - range.begin;}

  static size_t begin(const SizeRange& range)
    {return range.begin;}

  static size_t end(const SizeRange& range)
    {return range.end;}

  static const size_t& value(const size_t& iterator)
    {return iterator;}

  static String toString(const SizeRange& range)
    {return T("[") + lbcpp::toString(range.begin) + T(", ") + lbcpp::toString(range.end) + T("[");}

  static void write(OutputStream& ostr, const SizeRange& range)
    {lbcpp::write(ostr, range.begin); lbcpp::write(ostr, range.end);}

  static bool read(InputStream& istr, const SizeRange& range)
    {return lbcpp::read(istr, range.begin) && lbcpp::read(istr, range.end);}
};


}; /* namespace lbcpp */

#endif // !LBCPP_CONTAINER_TRAITS_H_

