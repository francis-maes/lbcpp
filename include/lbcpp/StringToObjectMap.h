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
| Filename: StringToObjectMap.h            | String -> Object Map            |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 12:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_STRING_TO_OBJECT_MAP_H_
# define LBCPP_STRING_TO_OBJECT_MAP_H_

# include "Object.h"

namespace lbcpp
{

class StringToObjectMap : public Object
{
public:
  void setObject(const String& key, ObjectPtr object)
    {objects[key] = object;}
  
  virtual String toString() const
  {
    if (objects.empty())
      return T("No objects");
    else if (objects.size() == 1)
      return T("Object ") + objects.begin()->first + T(":\n\t") + objects.begin()->second->toString();
    else
    {
      String res = String((int)objects.size()) + T(" objects:\n");
      for (ObjectsMap::const_iterator it = objects.begin(); it != objects.end(); ++it)
        res += T("Object ") + it->first + T(":\n\t") + lbcpp::toString(it->second) + T("\n");
      return res;
    }
  }
  
  typedef std::map<String, ObjectPtr> ObjectsMap;
  
  ObjectsMap getObjects() const
    {return objects;}

protected:
  ObjectsMap objects;

  virtual bool load(InputStream& istr)
    {return lbcpp::read(istr, objects);}

  virtual void save(OutputStream& ostr) const
    {lbcpp::write(ostr, objects);}
};

typedef ReferenceCountedObjectPtr<StringToObjectMap> StringToObjectMapPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_STRING_TO_OBJECT_MAP_H_
