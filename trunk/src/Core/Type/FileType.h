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

class FileType : public StringType
{
public:
  FileType(const String& name, TypePtr baseType)
    : StringType(name, baseType) {}

  virtual String toShortString(const VariableValue& value) const
    {return getFile(value).getFileName();}

  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const
  {
    File file = context.getFile(value);
    if (file == File::nonexistent)
      return VariableValue(String::empty);
    else
      return VariableValue(file.getFullPathName());
  }
  
private:
  File getFile(const VariableValue& value) const
    {return isMissingValue(value) ? File::nonexistent : File(value.getString());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_FILE_H_
