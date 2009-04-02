/*-----------------------------------------.---------------------------------.
| Filename: DoubleVectorHelpers.hpp        | Helper function to manipulate   |
| Author  : Francis Maes                   |   sorted sparse arrays          |
| Started : 27/02/2009 21:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_DOUBLE_VECTOR_HELPERS_HPP_
# define LCPP_DOUBLE_VECTOR_HELPERS_HPP_

namespace lcpp
{

template<class ElementType>
struct SortedArrayHelper
{
  typedef std::vector< std::pair<size_t, ElementType> > VectorType;
  
  static void set(VectorType& v, size_t index, const ElementType& value)
    {get(v, index) = value;}
  
  static void remove(VectorType& v, size_t index)
  {
    int i = findIndex(v, index);
    if (i >= 0)
      v.erase(v.begin() + i);
  }
  
  static ElementType& insert(VectorType& v, size_t index, const ElementType& value)
  {
    if (v.empty() || index > v.back().first)
    {
      v.push_back(std::make_pair(index, value)); 
      return v.back().second;
    }
    else
    {
      int pos = dichotomicSearch(v, index, 0, v.size());
      assert(pos >= 0 && pos <= (int)v.size());
      assert(pos >= (int)v.size() || v[pos].first != index); // element should not already exists
      v.insert(v.begin() + pos, std::make_pair(index, value)); // new element
      return v[pos].second;
    }
  }
  
  static ElementType& get(VectorType& v, size_t index, const ElementType& defaultValue = ElementType())
  {
    int res = findIndex(v, index);
    if (res >= 0)
      return v[res].second;
    return insert(v, index, defaultValue);
  }
  
  static const ElementType* get(const VectorType& v, size_t index)
  {
    int res = findIndex(v, index);
    return res < 0 ? NULL : &v[res].second;
  }
  
  static int findIndex(const VectorType& v, size_t index)
  {
    if (v.empty())
      return -1;
    size_t res = dichotomicSearch(v, index, 0, v.size());
    return res < v.size() && v[res].first == index ? (int)res : -1;
  }

private:
  static int findFirstEntryGreaterOrEqual(const VectorType& v, size_t index, size_t startingPoint = 0)
  {
    //assertIntegrity();
    assert(startingPoint >= 0 && startingPoint <= v.size());
    if (startingPoint == v.size())
    {
      assert(v.empty() || v.back().first < index);
      return v.size();
    }
    size_t res = dichotomicSearch(index, startingPoint, v.size());
    assert(res >= startingPoint);
    assert(res <= v.size());
    return res;
  }
  
  static size_t dichotomicSearch(const VectorType& v, size_t index, size_t begin, size_t end)
  {
    assert(begin >= 0 && begin <= v.size() && end >= 0 && end <= v.size());
    assert(begin <= end);
    assert(begin == 0 || v[begin - 1].first < index);
    assert(end == v.size() || v[end].first >= index);

    unsigned result;
    if (begin < v.size() && v[begin].first >= index)
      result = begin;
    else
      switch (end - begin)
      {
      case 0:
        result = begin;
        break;
        
      case 1:
        assert(begin < v.size());
        result = v[begin].first >= index ? begin : (begin + 1);
        break;
        
      case 2:
        assert(begin + 1 < v.size());
        result = v[begin].first >= index ? begin :
          (v[begin + 1].first >= index ? (begin + 1) : (begin + 2));
        break;
        
      case 3:
      case 4:
      case 5:
        for (result = begin; result < end; ++result)
        {
          assert(result < v.size());
          if (v[result].first >= index)
            break;
        }
        break;
        
      default:
        {
          // estimate split point
/*          index_t beginIndex = begin < v.size() ? v[begin].first : v.back().first;
          index_t endIndex = end < v.size() ? v[end].first : v.back().first;
          double k = (index - beginIndex) / (double)(endIndex - beginIndex);
          unsigned split = (unsigned)(begin + k * (end - begin));
          
          // force to reduce from at least 2 candidates
          if (split < begin + 2) split = begin + 2;
          if (split > end - 2) split = end - 2;*/
          size_t split = (begin + end) / 2;
         
          assert(split < v.size());
          if (v[split].first >= index)
            result = dichotomicSearch(v, index, begin, split);
          else
            result = dichotomicSearch(v, index, split, end);
        }
      };
    assert(result == v.size() || v[result].first >= index);
    assert(result == 0 || v[result - 1].first < index);
    return result;
  }
};

typedef SortedArrayHelper<double> SortedFeatureArrayHelper;
typedef SortedArrayHelper<SparseVectorPtr> SortedSubVectorArrayHelper;

}; /* namespace lcpp */

#endif // !LCPP_DOUBLE_VECTOR_HELPERS_HPP_
