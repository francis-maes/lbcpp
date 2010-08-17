/*-----------------------------------------.---------------------------------.
| Filename: DirectoriesObjectPairStream.h  | Pair of directories => stream   |
| Author  : Francis Maes                   |  of ObjectPairs                 |
| Started : 06/05/2010 12:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_DIRECTORIES_OBJECT_PAIR_H_
# define LBCPP_OBJECT_STREAM_DIRECTORIES_OBJECT_PAIR_H_

# include <lbcpp/Object/ObjectStream.h>
# include <lbcpp/Object/ObjectPair.h>

namespace lbcpp
{

class DirectoriesObjectPairStream : public DirectoryObjectStream
{
public:
  DirectoriesObjectPairStream(const File& directory1, const File& directory2, const String& wildCardPattern = T("*"))
    : DirectoryObjectStream(directory1, wildCardPattern), directory2(directory2)
    {}
  DirectoriesObjectPairStream() {}

  virtual ObjectPtr parseFile(const File& file1)
  {
    File file2 = directory2.getChildFile(file1.getFileName());
    if (file2.exists())
    {
      ObjectPtr object1 = Object::createFromFile(file1);
      ObjectPtr object2 = Object::createFromFile(file2);
      if (object1 && object2)
        return new ObjectPair(file1.getFileNameWithoutExtension(), object1, object2);
    }
    return ObjectPtr();
  }

protected:
  File directory2;

  virtual bool load(InputStream& istr)
    {return DirectoryObjectStream::load(istr) && lbcpp::read(istr, directory2);}

  virtual void save(OutputStream& ostr) const
    {DirectoryObjectStream::save(ostr); lbcpp::write(ostr, directory2);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_DIRECTORIES_OBJECT_PAIR_H_
