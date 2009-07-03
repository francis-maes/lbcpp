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
**
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
** @brief Object function container for streaming.
**
** A function can be "converted" to an ObjectFunction and used throw a
** stream.
**
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
  ** @a function apply object function (named f(x)) to an object pointer.
  **
  ** @param object : an object pointer passed as function parameter.
  **
  ** @return f(ObjectPtr).
  */
  virtual ObjectPtr function(ObjectPtr object) const = 0;
};


/*!
** @class ObjectStream
** @brief Object stream.
**
** Some databases are too huge to fit into memory, so with
** ObjectStream, it is not necessary to load them entirely. A stream is
** open and items are load from the database only when necessary.
**
** In order to fit all usages, users can used ObjectStream as a stream
** (each new item should be load from the database using next()
** method) or as a data loader (all items are loaded into memory).
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
  ** Check if the stream is OK or not.
  **
  ** @return True, if the stream is valid. Returns False if any
  ** stream problem occurs.
  */
  virtual bool isValid() const
    {return true;}

  /*!
  ** Load and return the next item from the database.
  **
  ** @return an object pointer on the next database item.
  */
  virtual ObjectPtr next() = 0;

  /*!
  ** Load, cast and return the next item from the database.
  **
  ** @return an object pointer (of type @a T) on the next database item.
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> nextAndCast()
  {
    ObjectPtr res = next();
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /*!
  ** Check object content by its name.
  **
  ** @param expectedClassName : expected class name.
  **
  ** @return true if ContentClassName == expectClassName, false otherwise.
  */
  bool checkContentClassName(const std::string& expectedClassName);

  /*!
  ** Load object at the position currentPosition + @a maximumCount
  ** into the stream.
  **
  ** If @a maximunCount == 0, @a iterate() loads the last item and @a
  ** count == number of objects.
  **
  ** @param maximumCount : iteration steps.
  **
  ** @return always True.
  */
  bool iterate(size_t maximumCount = 0);

  /*!
  ** Load \a maximumCount items from the database into the stream. If
  ** @a maximumCount == 0, @a load() loads all items into memory.
  **
  ** @param maximumCount : number of item to load.
  **
  ** @return an object container pointer.
  */
  ObjectContainerPtr load(size_t maximumCount = 0);

  /*!
  ** Apply the function @a function to stream items.
  **
  ** @param function : function to apply on items.
  **
  ** @return a new object stream pointer.
  */
  ObjectStreamPtr apply(ObjectFunctionPtr function);
};

/*!
** Classification examples parser.
**
** @param filename : classification examples file name.
** @param features : feature dictionary.
** @param labels : label dictionary.
**
** @return a new object stream pointer.
*/
extern ObjectStreamPtr classificationExamplesParser(const std::string& filename,
                                FeatureDictionaryPtr features, StringDictionaryPtr labels);

// generateur d'exemples de classification linéairement séparable
/*!
** Classification examples generator (linear).
**
** @param numFeatures : feature vector dimension.
** @param numClasses : number of classes.
**
** @return a new object stream pointer.
*/
extern ObjectStreamPtr classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses);


/*!
**  Regression examples parser.
**
** @param filename : regression examples file name.
** @param features : feature dictionary.
**
** @return a new object stream pointer.
*/
extern ObjectStreamPtr regressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features);


/*!
** @class TextObjectParser
** @brief Text object parser.
**
** #FIXME
*/
class TextObjectParser : public ObjectStream
{
public:
  /*!
  ** Constructor.
  **
  ** @param filename : file name.
  **
  ** @return a TextObjectParser.
  */
  TextObjectParser(const std::string& filename);

  /*!
  ** Constructor.
  **
  ** @param newInputStream : input stream.
  **
  ** @return a TextObjectParser.
  */
  TextObjectParser(std::istream* newInputStream);

  /*!
  ** Destructor.
  */
  virtual ~TextObjectParser();

  /*!
  ** Check if the stream is OK or not.
  **
  ** @return True, if the stream is valid. Returns False if any
  ** stream problem occurs.
  */
  virtual bool isValid() const
    {return istr != NULL;}

  /*!
  ** #FIXME: what did you expect ?
  **
  */
  virtual void parseBegin()   {}

  /*!
  ** Parse a string line.
  **
  ** @param line : text line.
  **
  ** @return
  */
  virtual bool parseLine(const std::string& line) = 0;

  /*!
  **
  ** #FIXME: ?
  **
  ** @return
  */
  virtual bool parseEnd()     {return true;}

  /*!
  ** Load the next object in the stream (ie. the current object
  ** pointer is updated).
  **
  ** @return a pointer on the next object in the stream.
  */
  virtual ObjectPtr next();

protected:

  /*!
  ** currentObject setter.
  **
  ** @param object : object pointer.
  */
  void setResult(ObjectPtr object)
    {currentObject = object;}

  /*!
  ** Tokenize a text classification example and load it into
  ** the @a columns vector.
  **
  ** @param line : example text line.
  ** @param columns : item container.
  ** @param separators : item text separators ("\t" by default).
  */
  static void tokenize(const std::string& line,
                       std::vector< std::string >& columns,
                       const char* separators = " \t");

  /*!
  ** Generic input stream parser.
  **
  ** @param istr : input stream.
  ** @param res : result container.
  **
  ** @return True if OK, False otherwise.
  */
  template<class T>
  static bool parse(std::istream& istr, T& res)
    {return !(istr >> res).fail();}

  /*!
  ** Wrapper text -> input stream parse.
  **
  ** @param str : text to parse.
  ** @param res : result container.
  **
  ** @return True if OK, False otherwise.
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
** @brief Learning data object parser.
**
** #FIXME
*/
class LearningDataObjectParser : public TextObjectParser
{
public:

  /*!
  ** Constructor.
  **
  ** @param filename : learning data file name.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataObjectParser(const std::string& filename, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(filename), features(features) {}

  /*!
  ** Constructor
  **
  ** @param newInputStream : input stream.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataObjectParser(std::istream* newInputStream, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(newInputStream), features(features) {}

  /*!
  ** Parse an empty line.
  **
  ** @return Always True.
  */
  virtual bool parseEmptyLine()
    {return true;}

  /*!
  ** Parse a data line.
  **
  ** @param columns : data container.
  **
  ** @return False if any error occurs.
  */
  virtual bool parseDataLine(const std::vector<std::string>& columns)
    {return false;}

  /*!
  ** Parse comment line.
  **
  ** @param comment : text line.
  **
  ** @return always True.
  */
  virtual bool parseCommentLine(const std::string& comment)
    {return true;}

  /*!
  ** #FIXME ?
  **
  **
  ** @return parseEmptyLine().
  */
  virtual bool parseEnd()
    {return parseEmptyLine();}

  /*!
  ** Parse a text line.
  **
  ** @param line : text line.
  **
  ** @return False if any error occurs.
  */
  virtual bool parseLine(const std::string& line);

protected:
  FeatureDictionaryPtr features; /*!< A pointer to features dictionary.*/


  /*!
  ** featureList ::= feature featureList | feature
  **
  ** @param columns : feature container.
  ** @param firstColumn : start column number.
  ** @param res : parse result container.
  **
  ** @return False i any error occurs.
  */
  bool parseFeatureList(const std::vector<std::string>& columns,
                        size_t firstColumn, SparseVectorPtr& res);

  /*!
  ** Feature parser.
  **
  ** feature ::= featureId : featureValue
  **
  ** @param str : couple < featureId, featureValue >.
  ** @param featureId : feature ID container.
  ** @param featureValue : feature value container.
  **
  ** @return False is any problem occurs.
  */
  static bool parseFeature(const std::string& str, std::string& featureId, double& featureValue);

  /*!
  ** Feature identifier parser.
  **
  ** featureId ::= name . featureId  | name
  **
  ** @param identifier : feature identifier.
  ** @param path : feature identifier path container.
  **
  ** @return True.
  */
  static bool parseFeatureIdentifier(const std::string& identifier, std::vector<std::string>& path);
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_H_
