/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
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
| Filename: Table.h                        | Base class for tables           |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_TABLE_H_
# define LBCPP_TABLE_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class TableHeader : public Object
{
public:
  enum Type
  {
    integerType,
    doubleType,
    stringType
  };

  size_t getNumColumns() const
    {return columns.size();}

  std::string getColumnName(size_t index) const
    {assert(index < columns.size()); return columns[index].first;}

  Type getColumnType(size_t index) const
    {assert(index < columns.size()); return columns[index].second;}

  void addColumn(const std::string& name, const Type& type)
    {columns.push_back(std::make_pair(name, type));}

private:
  std::vector<std::pair<std::string, Type> > columns;
};

class Table : public Object
{
public:
  virtual TableHeaderPtr getHeader() const = 0;
  virtual size_t getNumRows() const = 0;
  size_t getNumColumns() const;

  virtual int getInteger(size_t rowNumber, size_t columnNumber) const
    {assert(false); return 0;}
  virtual double getDouble(size_t rowNumber, size_t columnNumber) const
    {assert(false); return 0.0;}
  virtual std::string getString(size_t rowNumber, size_t columnNumber) const
    {assert(false); return "";}

  std::string toString(size_t rowNumber, size_t columnNumber) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_TABLE_H_
