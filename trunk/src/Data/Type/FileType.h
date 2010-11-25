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
| Filename: FileType.h                     | File type                       |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_FILE_H_
# define LBCPP_OBJECT_TYPE_FILE_H_

# include "StringType.h"
# include <algorithm>

namespace lbcpp
{

class DirectoriesCache
{
public:
  const std::vector<File>& getSubFiles(const File& file)
  {
    static std::vector<File> empty;
    if (!file.isDirectory())
      return empty;
    juce::Time time = file.getLastModificationTime();
    DirectoriesMap::const_iterator it = m.find(file.getFullPathName());
    if (it == m.end() || it->second.first != time)
    {
      std::pair<juce::Time, std::vector<File> >& entry = m[file.getFullPathName()];
      entry.first = time;

      juce::OwnedArray<File> files;
      file.findChildFiles(files, File::findFilesAndDirectories, false);
      entry.second.resize(files.size());
      for (int i = 0; i < files.size(); ++i)
        entry.second[i] = *files[i];
      std::sort(entry.second.begin(), entry.second.end(), sortFile);
      return entry.second;
    }
    else
      return it->second.second;
  }

private:
  typedef std::map<String, std::pair<juce::Time, std::vector<File> > > DirectoriesMap;
  DirectoriesMap m;

  static bool sortFile(const File& file1, const File& file2)
  {
    if (file1.isDirectory() != file2.isDirectory())
      return file1.isDirectory();
    return file1.getFileName() < file2.getFileName();
  }
};

class FileType : public StringType
{
public:
  FileType(const String& name, TypePtr baseType)
    : StringType(name, baseType) {}

  virtual size_t getNumElements(const VariableValue& value) const
    {return getSubFiles(value).size();}

  virtual String getElementName(const VariableValue& value, size_t index) const
  {
    const std::vector<File>& subFiles = getSubFiles(value);
    return index < subFiles.size() ? subFiles[index].getFileName() : String::empty;
  }

  virtual Variable getElement(const VariableValue& value, size_t index) const
  {
    const std::vector<File>& subFiles = getSubFiles(value);
    return index < subFiles.size() ? Variable(subFiles[index]) : Variable::missingValue(fileType);
  }
  
  virtual String toShortString(const VariableValue& value) const
    {return getFile(value).getFileName();}

  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const
  {
    if (value == String::empty)
      return VariableValue(String::empty);
    File v = File::getCurrentWorkingDirectory().getChildFile(value);
    return VariableValue(v.getFullPathName());
  }
  
private:
  static DirectoriesCache cache;

  File getFile(const VariableValue& value) const
    {return isMissingValue(value) ? File::nonexistent : File(value.getString());}

  const std::vector<File>& getSubFiles(const VariableValue& value) const
    {return cache.getSubFiles(getFile(value));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_FILE_H_
