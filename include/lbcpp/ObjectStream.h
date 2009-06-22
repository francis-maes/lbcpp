/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.h                 | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


/*!
**@file   ObjectStream.h
**@author Francis MAES
**@date   Fri Jun 12 11:28:21 2009
**
**@brief  Declaration of Object Streams.
**
** Object streams are #FIXME: what is it ?
**
**
*/

#ifndef LBCPP_OBJECT_STREAM_H_
# define LBCPP_OBJECT_STREAM_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class ObjectFunction
** @brief #FIXME: aim ?
**
** #FIXME
*/
class ObjectFunction : public Object
{
public:
  /*!
  ** @a getOutputClassName returns Object class name.
  **
  ** @return class name.
  */
  virtual std::string getOutputClassName() const = 0;

  /*!
  ** @a function #FIXME: what does it do ?
  **
  ** @param object : #FIXME: what is it ?
  **
  ** @return Returns an ObjectPtr.
  */
  virtual ObjectPtr function(ObjectPtr object) const = 0;
};


/*!
** @class ObjectStream
** @brief #FIXME: aim ?
**
** #FIXME
*/
class ObjectStream : public Object
{
public:
  /*!
  ** Content class name getter.
  **
  ** @return content class name (std::string).
  */
  virtual std::string getContentClassName() const
    {return "Object";}

  /*!
  ** @a isValid checks if the stream is OK or not.
  **
  ** @return True, if the stream is valid. Returns False if any
  ** stream problem occurs.
  */
  virtual bool isValid() const
    {return true;}

  /*!
  ** #FIXME: what does it do ?
  **
  ** @return
  */
  virtual ObjectPtr next() = 0;

  /*!
  **
  **
  **
  ** @return
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> nextAndCast()
  {
    ObjectPtr res = next();
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /*!
  **
  **
  ** @param expectedClassName
  **
  ** @return
  */
  bool checkContentClassName(const std::string& expectedClassName);

  /*!
  **
  **
  ** @param maximumCount
  **
  ** @return
  */
  bool iterate(size_t maximumCount = 0);

  /*!
  **
  **
  ** @param maximumCount
  **
  ** @return
  */
  ObjectContainerPtr load(size_t maximumCount = 0);

  /*!
  **
  **
  ** @param function
  **
  ** @return
  */
  ObjectStreamPtr apply(ObjectFunctionPtr function);
};

/*!
**
**
** @param filename
** @param features
** @param labels
**
** @return
*/
extern ObjectStreamPtr classificationExamplesParser(const std::string& filename,
                                FeatureDictionaryPtr features, StringDictionaryPtr labels);

// generateur d'exemples de classification linéairement séparable
extern ObjectStreamPtr classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses);


/*!
**
**
** @param filename
** @param features
**
** @return
*/
extern ObjectStreamPtr regressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features);


/*!
** @class TextObjectParser
** @brief #FIXME
**
** #FIXME
*/
class TextObjectParser : public ObjectStream
{
public:
  /*!
  **
  **
  ** @param filename
  **
  ** @return
  */
  TextObjectParser(const std::string& filename);

  /*!
  **
  **
  ** @param newInputStream
  **
  ** @return
  */
  TextObjectParser(std::istream* newInputStream);

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~TextObjectParser();

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool isValid() const
    {return istr != NULL;}

  /*!
  **
  **
  */
  virtual void parseBegin()   {}

  /*!
  **
  **
  ** @param line
  **
  ** @return
  */
  virtual bool parseLine(const std::string& line) = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool parseEnd()     {return true;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual ObjectPtr next();

protected:

  /*!
  **
  **
  ** @param object
  */
  void setResult(ObjectPtr object)
    {currentObject = object;}

  /*!
  **
  **
  ** @param line
  ** @param columns
  */
  static void tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators = " \t");

  /*!
  **
  **
  ** @param istr
  ** @param res
  **
  ** @return
  */
  template<class T>
  static bool parse(std::istream& istr, T& res)
    {return !(istr >> res).fail();}

  /*!
  **
  **
  ** @param str
  ** @param res
  **
  ** @return
  */
  template<class T>
  static bool parse(const std::string& str, T& res)
    {std::istringstream istr(str); return parse(istr, res);}

private:
  ObjectPtr currentObject;      /*!< A pointer to the current Object. */
  std::istream* istr;           /*!< A pointer to the current stream. */
};


/*!
** @class LearningDataObjectParser
** @brief #FIXME
**
** #FIXME
*/
class LearningDataObjectParser : public TextObjectParser
{
public:

  /*!
  **
  **
  ** @param filename
  ** @param features
  **
  ** @return
  */
  LearningDataObjectParser(const std::string& filename, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(filename), features(features) {}

  /*!
  **
  **
  ** @param newInputStream
  ** @param features
  **
  ** @return
  */
  LearningDataObjectParser(std::istream* newInputStream, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(newInputStream), features(features) {}

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool parseEmptyLine()
    {return true;}

  /*!
  **
  **
  ** @param columns
  **
  ** @return
  */
  virtual bool parseDataLine(const std::vector<std::string>& columns)
    {return false;}

  /*!
  **
  **
  ** @param comment
  **
  ** @return
  */
  virtual bool parseCommentLine(const std::string& comment)
    {return true;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool parseEnd()
    {return parseEmptyLine();}

  /*!
  **
  **
  ** @param line
  **
  ** @return
  */
  virtual bool parseLine(const std::string& line);

protected:
  FeatureDictionaryPtr features; /*!< A pointer to features dictionary.*/

  // featureList ::= feature featureList | feature

  /*!
  **
  **
  ** @param columns
  ** @param firstColumn
  ** @param res
  **
  ** @return
  */
  bool parseFeatureList(const std::vector<std::string>& columns, size_t firstColumn, SparseVectorPtr& res);
  // feature ::= featureId . featureValue

  /*!
  **
  **
  ** @param str
  ** @param featureId
  ** @param featureValue
  **
  ** @return
  */
  static bool parseFeature(const std::string& str, std::string& featureId, double& featureValue);
    // featureId ::= name . featureId  | name

  /*!
  **
  **
  ** @param identifier
  ** @param path
  **
  ** @return
  */
  static bool parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path);
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_H_
