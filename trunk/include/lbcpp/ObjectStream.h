/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.h                 | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_OBJECT_STREAM_H_
# define LBCPP_OBJECT_STREAM_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class ObjectFunction : public Object
{
public:
  virtual std::string getOutputClassName() const = 0;
  virtual ObjectPtr function(ObjectPtr object) const = 0;
};

class ObjectStream : public Object
{
public:
  virtual std::string getContentClassName() const
    {return "Object";}

  virtual bool isValid() const
    {return true;}
    
  virtual ObjectPtr next() = 0;
  
  template<class T>
  inline ReferenceCountedObjectPtr<T> nextCast()
  {
    ObjectPtr res = next();
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  bool checkContentClassName(const std::string& expectedClassName);

  bool iterate(size_t maximumCount = 0);
  ObjectContainerPtr load(size_t maximumCount = 0);
  ObjectStreamPtr apply(ObjectFunctionPtr function);
};

extern ObjectStreamPtr classificationExamplesParser(const std::string& filename,
                                FeatureDictionaryPtr features, StringDictionaryPtr labels);

extern ObjectStreamPtr regressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features);

class TextObjectParser : public ObjectStream
{
public:
  TextObjectParser(const std::string& filename);
  TextObjectParser(std::istream* newInputStream);
  virtual ~TextObjectParser();
    
  virtual bool isValid() const
    {return istr != NULL;}
    
  virtual void parseBegin()   {}
  virtual bool parseLine(const std::string& line) = 0;
  virtual bool parseEnd()     {return true;}

  virtual ObjectPtr next();
  
protected:
  void setResult(ObjectPtr object)
    {currentObject = object;}

  static void tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators = " \t");

  template<class T>
  static bool parse(std::istream& istr, T& res)
    {return !(istr >> res).fail();}

  template<class T>
  static bool parse(const std::string& str, T& res)
    {std::istringstream istr(str); return parse(istr, res);}

private:
  ObjectPtr currentObject;
  std::istream* istr;
};

class LearningDataObjectParser : public TextObjectParser
{
public:
  LearningDataObjectParser(const std::string& filename, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(filename), features(features) {}
  LearningDataObjectParser(std::istream* newInputStream, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(newInputStream), features(features) {}
    
  virtual bool parseEmptyLine()
    {return true;}
  virtual bool parseDataLine(const std::vector<std::string>& columns)
    {return false;}
  virtual bool parseCommentLine(const std::string& comment)
    {return true;}
  virtual bool parseEnd()
    {return parseEmptyLine();}

  virtual bool parseLine(const std::string& line);
  
protected:
  FeatureDictionaryPtr features;

  // featureList ::= feature featureList | feature
  bool parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res);
  // feature ::= featureId . featureValue
  static bool parseFeature(const std::string& str, std::string& featureId, double& featureValue);
    // featureId ::= name . featureId  | name
  static bool parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path);
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_H_
