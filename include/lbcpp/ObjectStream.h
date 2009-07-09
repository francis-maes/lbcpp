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

/**
** @class ObjectFunction
** @brief Represents a function which takes an Object as input and
** returns an Object. ObjectFunction can be applied to Object streams
** and Object containers.
**
** A function object is a computer programming construct allowing an
** object to be invoked or called as if it were an ordinary function,
** usually with the same syntax.
**
** @see ObjectStream, ObjectContainer
*/
class ObjectFunction : public Object
{
public:
  /**
  ** Returns the class name of the function output.
  **
  ** @return the class name of the function output.
  */
  virtual std::string getOutputClassName() const = 0;

  /**
  ** @a function apply object function (named f(x)) to an object pointer.
  **
  ** @param object : an object pointer passed as function parameter.
  **
  ** @return f(ObjectPtr).
  */
  virtual ObjectPtr function(ObjectPtr object) const = 0;
};


/**
** @class ObjectStream
** @brief Object stream.
**
** Some databases are too huge to fit into memory, so with
** ObjectStream, it is not necessary to load them entirely. A stream is
** open and items are loaded from the database only when necessary.
**
** In order to fit all usages, users can use ObjectStream as a stream
** (each new item should be loaded from the database using next()
** method) or as a data loader (all items are loaded into memory).
*/
class ObjectStream : public Object
{
public:
  /**
  ** Content class name getter.
  **
  ** @return content class name (std::string).
  */
  virtual std::string getContentClassName() const
    {return "Object";}

  /**
  ** Checks if the stream is OK or not.
  **
  ** @return True, if the stream is valid. Returns False if any
  ** stream problem occurs.
  */
  virtual bool isValid() const
    {return true;}

  /**
  ** Loads and returns the next item from the stream.
  **
  ** @return an object pointer on the next database item.
  */
  virtual ObjectPtr next() = 0;

  /**
  ** Loads, casts and returns the next item from the stream.
  **
  ** @return an object pointer (of type @a T) on the next database item.
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> nextAndCast()
  {
    ObjectPtr res = next();
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /**
  ** Checks if getContentClassName() == @a expectedClassName.
  **
  ** @param expectedClassName : expected class name.
  **
  ** @return True if (ContentClassName == expectClassName), False
  ** otherwise and throw an error to ErrorManager.
  ** @see ObjectStream::getContentClassName
  ** @see Object::error
  */
  bool checkContentClassName(const std::string& expectedClassName) const;

  /**
  ** Calls next() up to @a maximumCount times and ignores the loaded
  ** objects. If @a maximumCount == 0, the function calls next() until
  ** the end of the stream. Otherwise, if the end of the stream occurs
  ** before @a maximumCount calls to next(), the function stops. Note
  ** that this function is essentially provided for debugging purpose.
  **
  **
  ** @param maximumCount : iteration steps.
  ** @return False if any errors occurs.
  */
  bool iterate(size_t maximumCount = 0);

  /**
  ** Loads \a maximumCount items (maximum) from the stream and stores
  ** them into memory. If
  ** @a maximumCount == 0, @a load() loads all items into memory.
  **
  ** @param maximumCount : number of item to load.
  **
  ** @return an object container instance containing loaded items.
  ** @see ObjectContainer
  */
  ObjectContainerPtr load(size_t maximumCount = 0);

  /**
  ** Applies the function @a function to a new stream derivated from the
  ** current stream.
  **
  ** @param function : function to apply on items.
  **
  ** @return a new object stream instance.
  */
  ObjectStreamPtr apply(ObjectFunctionPtr function);
};

/**
** Creates a classification examples parser.
**
** Each line of the datafile should contain a classification example.
** Each example begins with the class identifier followed by a list of
** features. A feature is defined by an identifier and an optional
** associated real value. When no value is specified, the default
** value 1.0 is assumed.
**
**     Examples:
**
**     - Dense Vectors, 6 continuous features, 2 classes:
**     \verbatim
**     1 1:0.0526316 2:0.2 3:-0.456954 4:-0.428571 5:-0.821918 6:-1
**     2 1:0.0526316 2:-0.286957 3:-0.271523 4:-0.298701 5:-0.876712
**     6:-1
**     2 1:0.105263 2:-0.46087 3:-0.615894 4:-0.714286 5:-0.664384
**     6:-1
**     \endverbatim
**
**     - Sparse Vectors, binary features, 3 classes:
**     \verbatim
**     yes 6:1 8:1 15:1 21:1 25:1 33:1 34:1 37:1 42:1 50:1 53:1 57:1
**     67:1 76:1 78:1 81:1 84:1 86:1
**     no 2:1 3:1 20:1 22:1 23:1 33:1 35:1 36:1 47:1 50:1 51:1 58:1
**     67:1 76:1 79:1 81:1 82:1 86:1
**     maybe 2:1 8:1 19:1 21:1 27:1 33:1 34:1 36:1 44:1 50:1 53:1 57:1
**     67:1 76:1 78:1 83:1 87:1
**     \endverbatim
**
**     - Equivalently, short syntax:
**     \verbatim
**     yes 6 8 15 21 25 33 34 37 42 50 53 57 67 76 78 81 84 86
**     no 2 3 20 22 23 33 35 36 47 50 51 58 67 76 79 81 82 86
**     maybe 2 8 19 21 27 33 34 36 44 50 53 57 67 76 78 83 87
**     \endverbatim
**
**     - Features do not need to be sorted and can use any
**     alphanumeric identifier:
**     \verbatim
**     class1 afeature anotherfeature
**     class2 feat150 feat12 feat315
**     class3 501636 23543 2353262
**     class4 aaa AAA bbb BBB
**     \endverbatim
**
**     - Both feature syntaxes can be mixed:
**     \verbatim
**     -1 1:0.7 2 4 5:0.9
**     +1 1:0.3 2 3 5:-0.7
**     \endverbatim
**
** Note that the format used by libSVM is a particular case of the one
** presented below. A repository of example datafiles can be found
** here: http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets.
**
** @param filename : classification examples file name.
** @param features : feature dictionary.
** @param labels : label dictionary.
**
** @return a new object stream pointer.
*/
extern ObjectStreamPtr classificationExamplesParser(const std::string& filename,
                                FeatureDictionaryPtr features, StringDictionaryPtr labels);

/**
** Creates a synthetic generator of linearly separable classification
** data.
**
** Each time you call next() function, a new
** classification example is generated where each feature is drawed
** from a normal Gaussian distribution and class labels are drawed from
** range [0, @a numClasses[.
**
** @param numFeatures : feature vector dimension.
** @param numClasses : number of classes.
**
** @return a new object stream pointer.
** @see ObjectStream::next
*/
extern ObjectStreamPtr classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses);


/**
**  Creates a regression examples parser.
**
** @param filename : regression examples file name.
** @param features : feature dictionary.
**
** @return a new object stream pointer.
*/
extern ObjectStreamPtr regressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features);


/**
** @class TextObjectParser
** @brief Text object parser.
**
** Parses a file line by line. Calls parseBegin() the first time you call
** next() function, them it calls parseLine() function until setResult() function
** is called. When next() function reaches the end of file, it calls
** parseEnd() function.
**
** @see TextObjectParser::parseBegin
** @see TextObjectParser::parseLine
** @see TextObjectParser::parseEnd
** @see TextObjectParser::setResult
*/
class TextObjectParser : public ObjectStream
{
public:
  /**
  ** Constructor.
  **
  ** @param filename : file name.
  **
  ** @return a TextObjectParser.
  */
  TextObjectParser(const std::string& filename);

  /**
  ** Constructor.
  **
  ** @param newInputStream : new input stream.
  **
  ** @return a TextObjectParser.
  */
  TextObjectParser(std::istream* newInputStream);

  /**
  ** Destructor.
  */
  virtual ~TextObjectParser();

  /**
  ** Checks if the stream is OK or not.
  **
  ** @return True, if the stream is valid. Returns False if any
  ** stream problem occurs.
  */
  virtual bool isValid() const
    {return istr != NULL;}

  /**
  ** Function called at the begin of the parsing.
  **
  */
  virtual void parseBegin()   {}

  /**
  ** Parses a string line.
  **
  ** @param line : text line.
  **
  ** @return a boolean.
  */
  virtual bool parseLine(const std::string& line) = 0;

  /**
  **
  ** Function called at the end of the parsing.
  **
  ** @return a boolean.
  */
  virtual bool parseEnd()     {return true;}

  /**
  ** Loads the next object in the stream (ie. the current object
  ** pointer is updated).
  **
  ** @return a pointer on the next object in the stream or ObjectPtr()
  ** is there is no more object into the stream..
  */
  virtual ObjectPtr next();

protected:

  /**
  ** currentObject setter.
  **
  ** @param object : object pointer.
  */
  void setResult(ObjectPtr object)
    {currentObject = object;}

  /**
  ** Tokenizes a line.
  **
  ** @param line : example text line.
  ** @param columns : item container.
  ** @param separators : item text separators ("\t" by default).
  */
  static void tokenize(const std::string& line,
                       std::vector< std::string >& columns,
                       const char* separators = " \t");

  /**
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

  /**
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


/**
** @class LearningDataObjectParser
** @brief Base classe for data file parsers.
**
** Parsers using libSVM data file format.
**
** Each line of the datafile should contain a classification example.
** Each example begins with the class identifier followed by a list of
** features. A feature is defined by an identifier and an optional
** associated real value. When no value is specified, the default
** value 1.0 is assumed.
**
**     Examples:
**
**     - Dense Vectors, 6 continuous features, 2 classes:
**     \verbatim
**     1 1:0.0526316 2:0.2 3:-0.456954 4:-0.428571 5:-0.821918 6:-1
**     2 1:0.0526316 2:-0.286957 3:-0.271523 4:-0.298701 5:-0.876712
**     6:-1
**     2 1:0.105263 2:-0.46087 3:-0.615894 4:-0.714286 5:-0.664384
**     6:-1
**     \endverbatim
**
**     - Sparse Vectors, binary features, 3 classes:
**     \verbatim
**     yes 6:1 8:1 15:1 21:1 25:1 33:1 34:1 37:1 42:1 50:1 53:1 57:1
**     67:1 76:1 78:1 81:1 84:1 86:1
**     no 2:1 3:1 20:1 22:1 23:1 33:1 35:1 36:1 47:1 50:1 51:1 58:1
**     67:1 76:1 79:1 81:1 82:1 86:1
**     maybe 2:1 8:1 19:1 21:1 27:1 33:1 34:1 36:1 44:1 50:1 53:1 57:1
**     67:1 76:1 78:1 83:1 87:1
**     \endverbatim
**
**     - Equivalently, short syntax:
**     \verbatim
**     yes 6 8 15 21 25 33 34 37 42 50 53 57 67 76 78 81 84 86
**     no 2 3 20 22 23 33 35 36 47 50 51 58 67 76 79 81 82 86
**     maybe 2 8 19 21 27 33 34 36 44 50 53 57 67 76 78 83 87
**     \endverbatim
**
**     - Features do not need to be sorted and can use any
**     alphanumeric identifier:
**     \verbatim
**     class1 afeature anotherfeature
**     class2 feat150 feat12 feat315
**     class3 501636 23543 2353262
**     class4 aaa AAA bbb BBB
**     \endverbatim
**
**     - Both feature syntaxes can be mixed:
**     \verbatim
**     -1 1:0.7 2 4 5:0.9
**     +1 1:0.3 2 3 5:-0.7
**     \endverbatim
**
** Note that the format used by libSVM is a particular case of the one
** presented below. A repository of example datafiles can be found
** here: http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets.
**
*/
class LearningDataObjectParser : public TextObjectParser
{
public:

  /**
  ** Constructor.
  **
  ** @param filename : learning data file name.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataObjectParser(const std::string& filename, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(filename), features(features) {}

  /**
  ** Constructor
  **
  ** @param newInputStream : new input stream.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataObjectParser(std::istream* newInputStream, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(newInputStream), features(features) {}

  /**
  ** Parses an empty line.
  **
  ** @return a boolean.
  */
  virtual bool parseEmptyLine()
    {return true;}

  /**
  ** Parses a tokenized data line.
  **
  ** @param columns : tokenized data line.
  **
  ** @return a boolean.
  */
  virtual bool parseDataLine(const std::vector<std::string>& columns)
    {return false;}

  /**
  ** Parses comment line (a line starting by '#').
  **
  ** @param comment : comment line.
  **
  ** @return a boolean.
  */
  virtual bool parseCommentLine(const std::string& comment)
    {return true;}

  /**
  ** @a parseEnd is called when the parser reaches the end of file. It
  ** calls parseEmptyLine().
  **
  ** @return parseEmptyLine() result.
  ** @see LearningDataObjectParser::parseEmptyLine
  */
  virtual bool parseEnd()
    {return parseEmptyLine();}

  /**
  ** Parses a text line following these rules:
  ** - if the line is empty, it calls @a parseEmptyLine()
  ** - if the line starts by '#', it calls @a parseCommentLine()
  ** - otherwise, it tokenizes the line before calling @a parseDataLine()
  **
  **
  ** @param line : text line.
  **
  ** @return a boolean.
  ** @see LearningDataObjectParser::parseEmptyLine
  ** @see LearningDataObjectParser::parseCommentLine
  ** @see LearningDataObjectParser::parseDataLine
  */
  virtual bool parseLine(const std::string& line);

protected:
  FeatureDictionaryPtr features; /*!< A pointer to features dictionary.*/


  /**
  ** featureList ::= feature featureList | feature
  **
  ** @param columns : feature container.
  ** @param firstColumn : start column number.
  ** @param res : parse result container.
  **
  ** @return False if any error occurs.
  */
  bool parseFeatureList(const std::vector<std::string>& columns,
                        size_t firstColumn, SparseVectorPtr& res);

  /**
  ** Feature parser.
  **
  ** feature ::= featureId : featureValue
  **
  ** @param str : pair < featureId, featureValue >.
  ** @param featureId : feature ID container.
  ** @param featureValue : feature value container.
  **
  ** @return False if any problem occurs.
  */
  static bool parseFeature(const std::string& str, std::string& featureId, double& featureValue);

  /**
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
