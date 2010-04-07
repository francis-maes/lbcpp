/*-----------------------------------------.---------------------------------.
| Filename: StringDictionary.cpp           | String dictionary               |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2010 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/StringDictionary.h>
#include <lbcpp/Table.h>
using namespace lbcpp;

StringDictionary::StringDictionary(const StringDictionary& otherDictionary)
  : stringToIndex(otherDictionary.stringToIndex), indexToString(otherDictionary.indexToString)
{
}

StringDictionary::StringDictionary(const String& name, const juce::tchar* strings[])
  : NameableObject(name)
{
  for (int i = 0; strings[i]; ++i)
    add(strings[i]);
}

bool StringDictionary::exists(size_t index) const
  {return index < indexToString.size();}

int StringDictionary::getIndex(const String& str) const
{
  StringToIndexMap::const_iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
    return -1;
  else
    return (size_t)it->second;
}

size_t StringDictionary::add(const String& str)
{
  StringToIndexMap::iterator it = stringToIndex.find(str);
  if (it == stringToIndex.end())
  {
    size_t res = indexToString.size();
    indexToString.push_back(str);
    stringToIndex[str] = res;
    jassert(stringToIndex.find(str) != stringToIndex.end());
    return res;
  }
  else
    return it->second;
}

String StringDictionary::getString(size_t index) const
{
  if (index >= indexToString.size() || indexToString[index].isEmpty())
    return lbcpp::toString(index);
  else
    return indexToString[index];
}

void StringDictionary::save(OutputStream& ostr) const
{
  NameableObject::save(ostr);
  write(ostr, indexToString);
}

bool StringDictionary::load(InputStream& istr)
{
  clear();
  if (!NameableObject::load(istr) || !read(istr, indexToString))
    return false;

  // compute inverse map
  for (size_t i = 0; i < indexToString.size(); ++i)
    stringToIndex[indexToString[i]] = i;
  return true;
}


namespace lbcpp
{

  std::ostream& operator <<(std::ostream& ostr, const StringDictionary& strings)
  {
    ostr << "[";
    for (size_t i = 0; i < strings.indexToString.size(); ++i)
    {
      ostr << (const char* )strings.indexToString[i]; // << " [" << i << "]";
      if (i < strings.indexToString.size() - 1)
        ostr << ", ";
    }
    ostr << "]";
    return ostr;
  }

}; /* namespace lbcpp */

class StringDictionaryTable : public Table
{
public:
  StringDictionaryTable(StringDictionaryPtr dictionary)
    : dictionary(dictionary), header(new TableHeader())
  {
    header->addColumn("index", TableHeader::integerType);
    header->addColumn("string", TableHeader::stringType);
  }
    
  virtual TableHeaderPtr getHeader() const
    {return header;}
    
  virtual size_t getNumRows() const
    {return dictionary->getNumElements();}

  virtual int getInteger(size_t rowNumber, size_t columnNumber) const
    {jassert(columnNumber == 0); return (int)rowNumber;}
  
  virtual String getString(size_t rowNumber, size_t columnNumber) const
    {jassert(columnNumber == 1); return dictionary->getString(rowNumber);}
  
private:
  StringDictionaryPtr dictionary;
  TableHeaderPtr header;
};

TablePtr StringDictionary::toTable() const
{
  return new StringDictionaryTable(const_cast<StringDictionary* >(this));
}
