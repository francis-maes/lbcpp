/*-----------------------------------------.---------------------------------.
| Filename: FileTreeView.h                 | File Tree View                  |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2012 11:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_FILE_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_FILE_H_

# include "GenericTreeView.h"

namespace lbcpp
{

class FileTreeView : public GenericTreeView
{
public:
  FileTreeView(FilePtr file, const string& name)
    : GenericTreeView(file, name)
    {buildTree();}

  virtual bool mightHaveSubObjects(const ObjectPtr& object)
    {return object.isInstanceOf<Directory>();}

  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object)
  {
    DirectoryPtr directory = object.staticCast<Directory>();
    std::vector<FilePtr> files = directory->findFiles();
    std::vector< std::pair<string, ObjectPtr> > res(files.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = std::make_pair(files[i]->toShortString(), files[i]);
    return res;
  }

  virtual size_t getNumDataColumns()
    {return 2;}

  virtual std::vector<ObjectPtr> getObjectData(const ObjectPtr& object)
  {
    std::vector<ObjectPtr> res(2);
    FilePtr file = object.staticCast<File>();
    LoaderPtr loader = lbcpp::getTopLevelLibrary()->findLoaderForFile(defaultExecutionContext(), file->get());
    if (loader)
      res[0] = new String(loader->getTargetClass()->getName());
    if (!file.isInstanceOf<Directory>())
      res[1] = new MemorySize((size_t)file->get().getSize());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_FILE_H_
