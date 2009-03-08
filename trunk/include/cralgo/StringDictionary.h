/*-----------------------------------------.---------------------------------.
| Filename: StringDictionary.h             | A dictionary of strings         |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 19:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STRING_DICTIONARY_H_
# define CRALGO_STRING_DICTIONARY_H_

# include <map>
# include <vector>
# include <string>
# include <iostream>

namespace cralgo
{

class StringDictionary
{
public:
  void clear()
    {stringToIndex.clear(); indexToString.clear();}

  size_t count() const
    {return (unsigned)indexToString.size();}

  bool exists(size_t index) const;
  std::string getString(size_t index) const;
  
  // returns -1 if not found
  int getIndex(const std::string& str) const;
  
  size_t add(const std::string& str);

  friend std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings);

protected:
  typedef std::map<std::string, size_t> StringToIndexMap;
  typedef std::vector<std::string> StringVector;
 
  StringToIndexMap stringToIndex;
  StringVector indexToString;
};

}; /* namespace cralgo */

#endif // !CRALGO_STRING_DICTIONARY_H_
