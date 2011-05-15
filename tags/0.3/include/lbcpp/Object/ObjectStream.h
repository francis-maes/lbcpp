/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: ObjectStream.h                 | Object Streams                  |
| Author  : Francis Maes                   |                                 |
| Started : 08/06/2009 14:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_OBJECT_STREAM_H_
# define LBCPP_OBJECT_STREAM_H_

# include "../ObjectPredeclarations.h"

namespace lbcpp
{

/**
** @class ObjectFunction
** @brief Represents a function which takes an Object as input and
** returns an Object.
**
** A function object is a computer programming construct allowing an
** object to be invoked or called as if it were an ordinary function.
** ObjectFunction can be applied to Object streams
** and Object containers.
**
** @see ObjectStream, ObjectContainer
*/
class ObjectFunction : public NameableObject
{
public:
  ObjectFunction(const String& name) : NameableObject(name) {}
  ObjectFunction() : NameableObject() {}

  virtual String getInputClassName() const
    {return T("Object");}

  /**
  ** Returns the class name of the function's output.
  **
  ** @param inputClassName : the input's class name.
  ** @return the class name of the function's output.
  */
  virtual String getOutputClassName(const String& inputClassName) const = 0;

  /**
  ** Applies object function to an object.
  **
  ** @param object : an object pointer passed as function parameter.
  **
  ** @return an object of class getOutputClassName().
  */
  virtual ObjectPtr function(ObjectPtr object) const = 0;
};


/**
** @class ObjectStream
** @brief Object stream.
**
** This class represents a finite or infinite stream of objects.
** It is used as a base class for parsers and generators of data.
**
** ObjectStream is particurlaly usefull for large-scale learning
** where the whole dataset does not fit in memory.
** With an ObjectStream, it is possible to apply learning,
** by loading the training examples one at a time.
**
** In order to fit all usages, users can load the whole content
** of an ObjectStream  into memory, to have a random access on
** its elements.
*/
class ObjectStream : public NameableObject
{
public:
  ObjectStream(const String& name)
    : NameableObject(name) {}
  ObjectStream() {}

  /**
  ** Returns the class name of the objects contained by this stream.
  **
  ** If objects from multiple classes are mixed in this stream, this
  ** functions returns the highest base-class that is common between
  ** these classes.
  **
  ** @return content class name (String).
  */
  virtual String getContentClassName() const
    {return "Object";}

  /**
  ** Rewind the stream.
  **
  ** @return false if rewind is not supported on this stream
  */
  virtual bool rewind()
    {return false;}

  /**
  ** Checks if the stream is exhausted.
  **
  ** @return true if the stream is exhausted.
  */
  virtual bool isExhausted() const
    {return true;}

  /**
  ** Loads and returns the next element from the stream.
  **
  ** @return an object pointer on the next database element.
  */
  virtual ObjectPtr next() = 0;

  /**
  ** Loads, casts and returns the next element from the stream.
  **
  ** @return an object pointer (of type @a T) on the next database element.
  */
  template<class T>
  inline ReferenceCountedObjectPtr<T> nextAndCast()
  {
    ObjectPtr res = next();
    return res ? res.staticCast<T>() : ReferenceCountedObjectPtr<T>();
  }

  /**
  ** Calls next() up to @a maximumCount times and ignores the loaded
  ** objects.
  **
  ** If @a maximumCount == 0, the function calls next() until
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
  ** them into memory.
  **
  ** If @a maximumCount == 0, all the items will be loaded.
  **
  ** @param maximumCount : number of item to load.
  **
  ** @return an object container instance containing loaded items.
  ** @see ObjectContainer
  */
  ObjectContainerPtr load(size_t maximumCount = 0);

  /**
  ** Applies an ObjectFuntion to this stream.
  **
  ** This function creates a new stream derivated from this one.
  ** Each time next() is called on this stream, a new element
  ** is loaded from this one and @a function is applied on
  ** this element.
  **
  ** @param function : function to apply on elements.
  ** @return a new object stream instance referring to this one.
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
** @return a new ObjectStream containing classification examples.
*/
//extern ObjectStreamPtr classificationExamplesParser(const File& file,
 //                               FeatureDictionaryPtr features, FeatureDictionaryPtr labels);

/**
** Creates a synthetic generator of linearly separable classification
** data.
**
** Each time you call next() function, a new
** classification example is generated where each feature is drawed
** from a normal Gaussian distribution and class labels are drawed from
** range [0, @a numClasses[ in order to make the problem linearly separable.
**
** In order to ensure linear separability, vectors of parameters
** are drawn from a normal Gaussian distribution for each class,
** during construction of the stream.
**
** @param numFeatures : feature vector dimension.
** @param numClasses : number of classes.
**
** @return a new ObjectStream containing classification examples.
** @see ObjectStream::next
*/
//extern ObjectStreamPtr classificationExamplesSyntheticGenerator(size_t numFeatures, size_t numClasses);


/**
**  Creates a regression examples parser.
**
**  Each line of the datafile should contain a regression example. Each example
**  begins with the output scalar value followed by a list of input features.
**  As in classification data files, a feature is defined by an identifier and
**  an optional associated real value. When no feature value is specified,
**  the default value 1.0 is assumed.
**
**  Examples:
**
**  - Dense Vectors, 8 continuous features:
**  \verbatim
**  20 1:0.2 2:0.22973 3:0.210084 4:-0.734513 5:-0.450682 6:-0.682582 7:-0.628703 8:-0.345291
**  16 1:0.6 2:0.27027 3:0.243697 4:-0.778761 5:-0.457411 6:-0.605918 7:-0.607637 8:-0.484803
**  9 1:-1.2 2:0.0810811 3:0.0588235 4:-0.778761 5:-0.640517 6:-0.710155 7:-0.705069 8:-0.674141
**  \endverbatim
**
**  - Sparse Vectors, binary features:
**  \verbatim
**  0.26 6 8 15 21 29 33 34 37 42 50 53 57 67 76 78 81 84 86 93 103 111
**  0.72 6 8 20 21 23 33 34 36 42 50 53 57 67 76 78 81 84 86 95 102 107
**  0.35 2 8 19 21 27 33 34 36 44 50 53 57 67 76 78 81 84 86 95 102 108
**  \endverbatim
**
**  - Sparse Vectors, alphanumeric identifiers:
**  \verbatim
**  0.9 afeature anotherfeature
**  0.1 feat150 feat12 feat315
**  0.2 501636 23543 2353262
**  0.8 aaa AAA bbb BBB
**  \endverbatim
**
**  @param filename : The file containing regression data.
**  @param features : The dictionary of features.
**  @return a new ObjectStream containing regression examples.
**  @see classificationExamplesParser
**
*/
//extern ObjectStreamPtr regressionExamplesParser(const File& file, FeatureDictionaryPtr features);


/**
** @class TextObjectParser
** @brief Text object parser.
**
** Base class for parsing text files line by line.
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
  TextObjectParser(const File& file);

  /**
  ** Constructor.
  **
  ** @param newInputStream : an input stream.
  ** The TextObjectParser is responsible for deleting this
  ** stream, when no more used.
  **
  ** @return a TextObjectParser.
  */
  TextObjectParser(InputStream* newInputStream);

  /**
  ** Destructor.
  */
  virtual ~TextObjectParser();

  virtual bool rewind()
    {return istr->setPosition(0);}

  virtual bool isExhausted() const
    {return istr == NULL;}

  /**
  ** This function is called at the begging of parsing.
  **
  */
  virtual void parseBegin()   {}

  /**
  ** This function is called to process one line during parsing.
  **
  ** If an object has been fully loaded thanks to this line,
  ** call setResult() to complete the parsing of this object.
  **
  ** @param line : text line.
  **
  ** @return False if parsing of the line failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  ** @see setResult
  */
  virtual bool parseLine(const String& line) = 0;

  /**
  ** This function is called at the end of the parsing.
  **
  ** This function may call setResult(), if an object
  ** has been fully loaded thanks to the end of parsing.
  **
  ** @return False if end of parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  ** @see setResult
  */
  virtual bool parseEnd()     {return true;}

  /**
  ** Loads the next object from the stream.
  **
  ** When called the first time next() calls parseBegin().
  ** In order to load the next object from the stream, next()
  ** calls parseLine() for each line to parse until
  ** a result is set through a call to setResult(). This result
  ** is then returned. When reaching end-of-file, this
  ** function calls parseEnd().
  **
  ** @return a pointer on the next parsed object or ObjectPtr()
  ** if there are no more object in the stream.
  */
  virtual ObjectPtr next();

protected:
  /**
  ** currentObject setter.
  **
  ** This function may be called by parseLine() and by parseEnd()
  ** to transmit the lastly parsed object, which will be returned
  ** by next().
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
  ** @param separators : item text separators (" " and "\t" by default).
  */
  static void tokenize(const String& line,
                       std::vector< String >& columns,
                       const juce::tchar* separators = T(" \t"));

private:
  ObjectPtr currentObject;      /*!< A pointer to the current Object. */
  InputStream* istr;           /*!< A pointer to the current stream. */
};


/**
** @class LearningDataObjectParser
** @brief Base classe for data parsers using libSVM inspired formats.
**
** @see classificationExamplesParser, regressionExamplesParser
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
  LearningDataObjectParser(const File& file, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(file), features(features) {}

  /**
  ** Constructor
  **
  ** @param newInputStream : new input stream. This object is
  ** responsible for deleting the input stream, when no longer used.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataObjectParser(InputStream* newInputStream, FeatureDictionaryPtr features = FeatureDictionaryPtr())
    : TextObjectParser(newInputStream), features(features) {}

  /**
  ** This function is called to parse an empty line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseEmptyLine()
    {return true;}

  /**
  ** This function is called to parse a line containing data.
  **
  ** @param columns : tokenized data line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseDataLine(const std::vector<String>& columns)
    {return false;}

  /**
  ** This function is called to parse a comment line (starting by '#').
  **
  ** @param comment : the comment (the line without '#')
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseCommentLine(const String& comment)
    {return true;}

  /**
  ** This function is called when parsing finishes.
  **
  ** By default, end-of-file is considered as an empty line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseEnd()
    {return parseEmptyLine();}

  /**
  ** Parses one line.
  **
  ** The following rules are applied:
  ** - if the line is empty, parseLine() calls parseEmptyLine()
  ** - if the line starts by '#', it calls parseCommentLine()
  ** - otherwise, it tokenizes the line and calls parseDataLine()
  **
  ** @param line : text line.
  **
  ** @return a boolean.
  ** @see LearningDataObjectParser::parseEmptyLine
  ** @see LearningDataObjectParser::parseCommentLine
  ** @see LearningDataObjectParser::parseDataLine
  */
  virtual bool parseLine(const String& line);

protected:
  FeatureDictionaryPtr features; /*!< A pointer to features dictionary.*/

  /**
  ** Parses a list of features.
  **
  ** Feature lists have the following grammar:
  ** \verbatim featureList ::= feature featureList | feature \endverbatim
  **
  ** @param columns : the columns to parse.
  ** @param firstColumn : start column number.
  ** @param res : parse result container.
  **
  ** @return False if any error occurs.
  */
  bool parseFeatureList(const std::vector<String>& columns,
                        size_t firstColumn, SparseVectorPtr& res);

  /**
  ** Parses a feature
  **
  ** Features have the following grammar:
  ** \verbatim feature ::= featureId : featureValue | featureId \endverbatim
  **
  ** @param str : the string to parse.
  ** @param featureId : feature ID container.
  ** @param featureValue : feature value container.
  **
  ** @return False if any error occurs.
  */
  static bool parseFeature(const String& str, String& featureId, double& featureValue);

  /**
  ** Parses a feature identifier
  **
  ** Feature identifiers have the following grammar:
  ** \verbatim featureId ::= name . featureId  | name \endverbatim
  **
  ** @param identifier : the string to parse.
  ** @param path : feature identifier path container.
  **
  ** @return True.
  */
  static bool parseFeatureIdentifier(const String& identifier, std::vector<String>& path);
};

class DirectoryObjectStream : public ObjectStream
{
public:
  DirectoryObjectStream(const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);
  DirectoryObjectStream();

  virtual bool rewind();
  virtual bool isExhausted() const;
  virtual ObjectPtr next();

protected:
  virtual ObjectPtr parseFile(const File& file);

  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;

private:
  File directory;
  String wildCardPattern;
  bool searchFilesRecursively;

  juce::OwnedArray<File> files;
  int nextFilePosition;

  void initialize();
};

extern ObjectStreamPtr directoriesObjectPairStream(const File& directory1, const File& directory2, const String& wildCardPattern = T("*"));

inline ObjectStreamPtr directoryObjectStream(const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false)
  {return new DirectoryObjectStream(directory, wildCardPattern, searchFilesRecursively);}

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_H_