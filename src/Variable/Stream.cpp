/*-----------------------------------------.---------------------------------.
| Filename: Stream.cpp                     | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Stream.h>
#include <lbcpp/Object/Vector.h>
using namespace lbcpp;

/*
** Stream
*/
ContainerPtr Stream::load(size_t maximumCount)
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

StreamPtr Stream::apply(FunctionPtr function) const
  {return StreamPtr();}//new ApplyFunctionStream(this, function);}

/*
** DirectoryFileStream
*/
DirectoryFileStream::DirectoryFileStream(const File& directory, const String& wildCardPattern, bool searchFilesRecursively)
  : directory(directory), wildCardPattern(wildCardPattern), searchFilesRecursively(searchFilesRecursively)
  {initialize();}

DirectoryFileStream::DirectoryFileStream() : nextFilePosition(0) {}

void DirectoryFileStream::initialize()
{
  directory.findChildFiles(files, File::findFiles, searchFilesRecursively, wildCardPattern);
  nextFilePosition = 0;
}

bool DirectoryFileStream::isExhausted() const
  {return nextFilePosition >= files.size();}

bool DirectoryFileStream::rewind()
{
  nextFilePosition = 0;
  return true;
}

Variable DirectoryFileStream::next()
{
  if (isExhausted())
    return Variable();
  jassert(nextFilePosition < files.size());
  File file = *files[nextFilePosition];
  ++nextFilePosition;
  return Variable(file.getFullPathName(), fileType());
}

void declareStreamClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Stream, Object);
  LBCPP_DECLARE_CLASS(DirectoryFileStream, Stream);
}
