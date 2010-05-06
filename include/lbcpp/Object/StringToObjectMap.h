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

class StringToObjectMap;
typedef ReferenceCountedObjectPtr<StringToObjectMap> StringToObjectMapPtr;

class StringToObjectMap : public NameableObject
{
public:
  StringToObjectMap(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  virtual String toString() const
  {
    String res = getClassName() + T(" ") + name + T(":\n");
    if (objects.empty())
      res += T("No objects");
    else if (objects.size() == 1)
      res += T("Object ") + objects.begin()->first + T(":\n\t") + objects.begin()->second->toString();
    else
    {
      res += String((int)objects.size()) + T(" objects:\n");
      for (ObjectsMap::const_iterator it = objects.begin(); it != objects.end(); ++it)
        res += T("Object ") + it->first + T(":\n\t") + lbcpp::toString(it->second) + T("\n");
    }
    return res;
  }

  void removeObject(const String& key)
    {objects.erase(key);}

  void setObject(const String& key, ObjectPtr object)
  {
    objects[key] = object;
    jassert(key == object->getName());
  }

  void setObject(ObjectPtr object)
    {if (object) {objects[object->getName()] = object;}}

  ObjectPtr getObject(const String& key) const
  {
    ObjectsMap::const_iterator it = objects.find(key);
    return it == objects.end() ? ObjectPtr() : it->second;
  }

  typedef std::map<String, ObjectPtr> ObjectsMap;
  
  ObjectsMap getObjects() const
    {return objects;}
    
  std::vector<String> getKeys() const
  {
    std::vector<String> res;
    res.reserve(objects.size());
    for (ObjectsMap::const_iterator it = objects.begin(); it != objects.end(); ++it)
      res.push_back(it->first);
    return res;
  }

  virtual ObjectPtr clone() const
  {
    StringToObjectMapPtr res = Object::createAndCast<StringToObjectMap>(getClassName());
    res->name = name;
    res->objects = objects;
    return res;
  }

protected:
  ObjectsMap objects;

  virtual bool load(InputStream& istr)
  {
    if (!NameableObject::load(istr) || !lbcpp::read(istr, objects))
      return false;
    return true;
  }

  virtual void save(OutputStream& ostr) const
    {NameableObject::save(ostr); lbcpp::write(ostr, objects);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_STRING_TO_OBJECT_MAP_H_
