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

# include "../ObjectPredeclarations.h"
# include "Function.h"

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
  Stream() {}

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
  ** @return an object container instance containing loaded items.
  ** @see Container
  */
  ContainerPtr load(size_t maximumCount = 0);

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
};

class DirectoryFileStream : public Stream
{
public:
  DirectoryFileStream(const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);
  DirectoryFileStream();

  virtual TypePtr getElementsType() const
    {return fileType();}

  virtual bool rewind();
  virtual bool isExhausted() const;
  virtual Variable next();

private:
  File directory;
  String wildCardPattern;
  bool searchFilesRecursively;

  juce::OwnedArray<File> files;
  int nextFilePosition;

  void initialize();
};

inline StreamPtr directoryFileStream(const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false)
  {return new DirectoryFileStream(directory, wildCardPattern, searchFilesRecursively);}

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_STREAM_H_
