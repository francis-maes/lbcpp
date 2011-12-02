/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2010 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: Stream.h                       | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_VARIABLE_STREAM_H_
# define LBCPP_VARIABLE_STREAM_H_

# include "predeclarations.h"
# include "../Core/Function.h"
# include "DoubleVector.h"

namespace lbcpp
{

/**
** @class Stream
** @brief Variable stream.
**
** This class represents a finite or infinite stream of variables.
** It is used as a base class for parsers and generators of data.
**
** Stream is particurlaly usefull for large-scale learning
** where the whole dataset does not fit in memory.
** With an Stream, it is possible to apply learning,
** by loading the training examples one at a time.
**
** In order to fit all usages, users can load the whole content
** of an Stream into memory, to have a random access on
** its elements.
*/
class Stream : public Object
{
public:
  Stream(ExecutionContext& context) : context(context) {}
  Stream() : context(*(ExecutionContext* )0) {}

  /**
  ** Returns the type of the elements contained by this stream.
  **
  ** If elements from multiple types are mixed in this stream, this
  ** functions returns the highest common base-type of elements.
  **
  ** @return the type of elements contained by this stream
  */
  virtual TypePtr getElementsType() const = 0;

  /**
  ** Rewind the stream.
  **
  ** @return false if rewind is not supported on this stream.
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

  virtual ProgressionStatePtr getCurrentPosition() const = 0;

  /**
  ** Loads and returns the next element from the stream.
  **
  ** @return a Variable containing the next database element.
  */
  virtual Variable next() = 0;


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
  ** @return a Vector containing loaded items.
  ** @see Container
  */
  VectorPtr load(size_t maximumCount = 0, bool doProgression = true);

  /**
  ** Applies an Function to this stream.
  **
  ** This function creates a new stream derivated from this one.
  ** Each time next() is called on this stream, a new element
  ** is loaded from this one and @a function is applied on
  ** this element.
  **
  ** @param function : function to apply on elements.
  ** @return a new object stream instance referring to this one.
  ** @see Function
  */
  StreamPtr apply(FunctionPtr function) const;

protected:
  ExecutionContext& context;
};

extern ClassPtr streamClass;

extern StreamPtr doubleStream(TypePtr elementsType, const std::vector<double>& values);
extern StreamPtr integerStream(TypePtr elementsType, const std::vector<int>& values);
extern StreamPtr booleanStream(const std::vector<bool>& values);
extern StreamPtr booleanStream(bool value);
extern StreamPtr objectStream(TypePtr elementsType, const std::vector<ObjectPtr>& values);

StreamPtr directoryFileStream(ExecutionContext& context, const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);
StreamPtr directoryPairFileStream(ExecutionContext& context, const File& mainDirectory, const File& secondDirectory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);

/**
 ** @class TextObjectParser
 ** @brief Text object parser.
 **
 ** Base class for parsing text files line by line.
 */
class TextParser : public Stream
{
public:
  /**
   ** Constructor.
   **
   ** @param filename : file name.
   **
   ** @return a TextObjectParser.
   */
  TextParser(ExecutionContext& context, const File& file);
  TextParser() {}

  /**
   ** Destructor.
   */
  virtual ~TextParser();
  
  virtual bool rewind();
  virtual bool isExhausted() const
    {return f == NULL;}

  virtual ProgressionStatePtr getCurrentPosition() const;
  
  /**
   ** This function is called at the begging of parsing.
   **
   */
  virtual void parseBegin()
    {}
  
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
  virtual bool parseLine(const String& line)
    {jassert(false); return false;}

  virtual bool parseLine(char* line)
    {return parseLine(String(line));}
  
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
  virtual bool parseEnd()
    {return true;}
  
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
   ** @return a pointer on the next parsed object or Variable()
   ** if there are no more object in the stream.
   */
  virtual Variable next();
  
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
  void setResult(const Variable& result)
    {currentResult = result;}
  
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
  Variable currentResult;      /*!< The current Variable. */
//  InputStream* istr;           /*!< A pointer to the current stream. */
  ProgressionStatePtr progression;

  FILE* f;

  char* line;
  int maxLineLength;
  size_t lineNumber;

  char* readNextLine();

  void initialize(InputStream* istr);
};

typedef ReferenceCountedObjectPtr<TextParser> TextParserPtr;

/* LibSVM Data Parser */
extern TextParserPtr regressionLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features);
extern TextParserPtr binaryClassificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features);
extern TextParserPtr classificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels);
extern TextParserPtr multiLabelClassificationLibSVMDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels);

extern StreamPtr binaryClassificationLibSVMFastParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features);

/* ARFF Data Parser */
extern TextParserPtr regressionARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features);
extern TextParserPtr binaryClassificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features);
extern TextParserPtr classificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels, bool sparseData = false);
extern TextParserPtr multiLabelClassificationARFFDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels);

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_STREAM_H_
