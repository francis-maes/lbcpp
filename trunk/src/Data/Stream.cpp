/*-----------------------------------------.---------------------------------.
| Filename: Stream.cpp                     | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Stream.h>
#include <lbcpp/Data/Vector.h>
using namespace lbcpp;

/*
** Stream
*/
VectorPtr Stream::load(size_t maximumCount)
{
  VectorPtr res = new Vector(getElementsType());
  while (maximumCount == 0 || res->size() < maximumCount)
  {
    Variable variable = next();
    if (variable)
      res->append(variable);
    else
      break;
  }
  return res;
}

bool Stream::iterate(size_t maximumCount)
{
  size_t count = 0;
  while (maximumCount == 0 || count < maximumCount)
  {
    Variable variable = next();
    if (variable)
      ++count;
    else
      break;
  }
  return true;
}

////

#include "Stream/DirectoryFileStream.h"
#include "Stream/ApplyFunctionStream.h"

StreamPtr lbcpp::directoryFileStream(const File& directory, const String& wildCardPattern, bool searchFilesRecursively)
  {return new DirectoryFileStream(directory, wildCardPattern, searchFilesRecursively);}

StreamPtr Stream::apply(FunctionPtr function) const
{
  return checkInheritance(getElementsType(), function->getInputType())
    ? StreamPtr(new ApplyFunctionStream(StreamPtr(const_cast<Stream* >(this)), function))
    : StreamPtr();
}

void declareStreamClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Stream, Object);
    LBCPP_DECLARE_CLASS(DirectoryFileStream, Stream);
}
