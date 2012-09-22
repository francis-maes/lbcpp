/*-----------------------------------------.---------------------------------.
| Filename: LibSVMDataParser.cpp           | LibSVMDataParser                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LibSVMDataParser.h"
using namespace lbcpp;

// FIXME: Duplicate with Stream.cpp ! This function must be incorporated into String class, no ?
inline int indexOfAnyNotOf(const String& str, const String& characters, int startPosition = 0)
{
  for (int i = startPosition; i < str.length(); ++i)
    if (characters.indexOfChar(str[i]) < 0)
      return i;
  return -1;
}

bool LibSVMDataParser::parseLine(const String& line)
{
  int begin = indexOfAnyNotOf(line, T(" \t"));
  bool isEmpty = begin < 0;
  if (isEmpty)
    return parseEmptyLine();
  if (line[begin] == '#')
    return parseCommentLine(line.substring(begin + 1).trim());
  std::vector<String> columns;
  tokenize(line, columns);
  return parseDataLine(columns);
}

SparseDoubleVectorPtr LibSVMDataParser::parseFeatureList(DefaultEnumerationPtr features, const std::vector<String>& columns, size_t firstColumn) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector(features, doubleType);
  for (size_t i = firstColumn; i < columns.size(); ++i)
  {
    String identifier;
    double value;
    if (!parseFeature(columns[i], identifier, value))
      return SparseDoubleVectorPtr();
    size_t index = features->findOrAddElement(context, identifier);
    res->setElement(index, value);
  }
  return res;
}

bool LibSVMDataParser::parseFeature(const String& str, String& featureId, double& featureValue)
{
  int n = str.indexOfChar(':');
  if (n < 0)
  {
    featureId = str;
    featureValue = 1.0;
  }
  else
  {
    featureId = str.substring(0, n);
    featureValue = str.substring(n + 1).getDoubleValue();
  }
  return true;
}
