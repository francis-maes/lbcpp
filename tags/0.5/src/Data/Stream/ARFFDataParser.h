/*-----------------------------------------.---------------------------------.
| Filename: ARFFDataParser.h               | ARFF Data Parser                |
| Author  : Julien Becker                  |                                 |
| Started : 10/07/2010 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_
# define LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

class ARFFDataParser : public TextParser
{
public:
  ARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, bool sparseData = false)
    : TextParser(context, file), features(features), sparseData(sparseData), shouldReadData(false) {}

  ARFFDataParser() {}
  
  virtual bool parseLine(const String& line);

protected:
  std::vector<TypePtr> attributesType;
  std::vector<String> attributesName;
  TypePtr supervisionType;
  DynamicClassPtr features;
  bool sparseData;
  bool shouldReadData;

  bool parseAttributeLine(const String& line);
  bool parseEnumerationAttributeLine(const String& line);
  bool checkOrAddAttributesTypeToFeatures();
  virtual bool checkSupervisionType() const = 0;
  bool parseDataLine(const String& line);
  bool parseSparseDataLine(const String& line);
  virtual Variable finalizeData(Variable data) const
    {return data;}
};

class RegressionARFFDataParser : public ARFFDataParser
{
public:
  RegressionARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features)
    : ARFFDataParser(context, file, features)
    {elementsType = pairClass(features, doubleType);}

  RegressionARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}
  
protected:
  TypePtr elementsType;
  
  virtual bool checkSupervisionType() const
    {return context.checkInheritance(supervisionType, doubleType);}
};

class BinaryClassificationARFFDataParser : public ARFFDataParser
{
public:
  BinaryClassificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features)
    : ARFFDataParser(context, file, features)
    {elementsType = pairClass(features, booleanType);}

  BinaryClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  
  virtual bool checkSupervisionType() const
  {
    if (!context.checkInheritance(supervisionType, enumValueType))
      return false;
    
    EnumerationPtr enumType = supervisionType.dynamicCast<Enumeration>();
    if (enumType->getNumElements() != 2)
    {
      context.errorCallback(T("BinaryClassificationARFFDataParser::checkSupervisionType"), T("Too many possible output"));
      return false;
    }
    return true;
  }
  
  virtual Variable finalizeData(Variable data) const
  {
    PairPtr p = data.getObjectAndCast<Pair>();
    p->setSecond(p->getSecond().getInteger() == 1);
    return p;
  }
};

class ClassificationARFFDataParser : public ARFFDataParser
{
public:
  ClassificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels, bool sparseData = false)
    : ARFFDataParser(context, file, features, sparseData), labels(labels)
    {elementsType = pairClass(features, labels);}

  ClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  DefaultEnumerationPtr labels;
  
  virtual bool checkSupervisionType() const;
};

class MultiLabelClassificationARFFDataParser : public ARFFDataParser
{
public:
  MultiLabelClassificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels)
    : ARFFDataParser(context, file, features), labels(labels)
    {elementsType = pairClass(features, sparseDoubleVectorClass(labels, probabilityType));}

  MultiLabelClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  DefaultEnumerationPtr labels;
  
  virtual bool checkSupervisionType() const;
};

};

#endif // !LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_
