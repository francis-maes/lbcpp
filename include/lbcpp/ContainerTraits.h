/*-----------------------------------------.---------------------------------.
| Filename: ContainerTraits.h              | C++ Containers Type Traits      |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 21:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ContainerTraits.h
**@author Francis MAES
**@date   Fri Jun 12 17:09:14 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_CONTAINER_TRAITS_H_
# define LBCPP_CONTAINER_TRAITS_H_

# include "Traits.h"
# include "Random.h"
# include <vector>
# include <set>
# include <map>
# include <limits>

# include <iostream> //tmp

namespace lbcpp
{
/*
Traits:
  static std::string toString(const T& container)
  static void write(std::ostream& ostr, const T& container)
  static bool read(std::istream& istr, T& container)

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
    assert(ContainerTraits::size(container));
    int r = Random::getInstance().sampleSize(ContainerTraits::size(container));
    int i = 0;
    typename ContainerTraits::ConstIterator it;
    for (it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it, ++i)
      if (i == r)
        return ContainerTraits::value(it);
    assert(false);
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
    assert(ContainerTraits::size(container));

    // not tested
    size_t b = ContainerTraits::begin(container);
    size_t e = ContainerTraits::end(container);
    assert(e > b);
    result = Random::getInstance().sampleSize(b, e);
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
  static inline std::string toString(const ContainerType& container)
  {
    std::string res;
    for (ConstIterator it = ContainerTraits::begin(container); it != ContainerTraits::end(container); ++it)
    {
      if (res.length() > 0)
        res += ", ";
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
  static inline void write(std::ostream& ostr, const ContainerType& container)
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
      assert(isNumberValid(score));
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
      assert(false);
    }

//    std::cout << "Num bests: " << bests.size();
//    ConstIterator it = Traits< std::vector<ConstIterator> >::sampleRandom(bests);
//    std::cout << " =sample> " << lbcpp::toString(ContainerTraits::value(it)) << std::endl;
    return Traits< std::vector<ConstIterator> >::sampleRandom(bests);
  }
};

/*!
** @class ContainerTraits
** @brief #FIXME
**
*/
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
  static const typename ContainerType::value_type& value(const typename ContainerType::const_iterator& iterator)
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
    assert(container.size());
    int r = Random::getInstance().sampleSize(container.size());
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
    assert(false);
    return *(typename ContainerType::value_type* )0;
  }
};

/*
** Vectors
*/
/*!
** @struct Traits
** @brief #FIXME
**
*/
template<class T>
struct Traits< std::vector<T> > : public BuiltinContainerTraits< std::vector<T> >
{
  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  static inline std::string toString(const std::vector<T>& vector)
    {return "[" + BuiltinContainerTraits< std::vector<T> >::toString(vector) + "]";}

  /*!
  **
  **
  ** @param istr
  ** @param res
  **
  ** @return
  */
  static inline bool read(std::istream& istr, std::vector<T>& res)
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

  /*!
  **
  **
  ** @param vector
  **
  ** @return
  */
  static const T& sampleRandom(const std::vector<T>& vector)
  {
    assert(vector.size());
    return vector[Random::getInstance().sampleSize(vector.size())];
  }
};

/*
** Set
*/
template<class T>
struct Traits< std::set<T> > : public BuiltinContainerTraits< std::set<T> >
{
  /*!
  **
  **
  ** @param container
  **
  ** @return
  */
  static inline std::string toString(const std::set<T>& container)
    {return "{" + BuiltinContainerTraits< std::set<T> >::toString(container) + "}";}
};

/*
** Map
*/
template<class KeyType, class ValueType>
struct Traits< std::map<KeyType, ValueType> >
  : public BuiltinContainerTraits< std::map<KeyType, ValueType> >
{
};

template<class KeyType, class ValueType>
struct Traits< std::multimap<KeyType, ValueType> >
  : public BuiltinContainerTraits< std::multimap<KeyType, ValueType> >
{
};


/*
** Pairs
*/
template<class T1, class T2>
struct Traits< std::pair<T1, T2> > : public BuiltinTypeTraits< std::pair<T1, T2> >
{
  static inline std::string toString(const std::pair<T1, T2>& value)
    {return "(" + Traits<T1>::toString(value.first) + ", " + Traits<T2>::toString(value.second) + ")";}
  static inline void write(std::ostream& ostr, const std::pair<T1, T2>& value)
  {
    Traits<T1>::write(ostr, value.first);
    Traits<T2>::write(ostr, value.second);
  }
  static inline bool read(std::istream& istr, std::pair<T1, T2>& result)
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

  static std::string toString(const SizeRange& range)
    {return "[" + lbcpp::toString(range.begin) + ", " + lbcpp::toString(range.end) + "[";}

  static void write(std::ostream& ostr, const SizeRange& range)
    {lbcpp::write(ostr, range.begin); lbcpp::write(ostr, range.end);}

  static bool read(std::istream& istr, const SizeRange& range)
    {return lbcpp::read(istr, range.begin) && lbcpp::read(istr, range.end);}
};


}; /* namespace lbcpp */

#endif // !LBCPP_CONTAINER_TRAITS_H_

