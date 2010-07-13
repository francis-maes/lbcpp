
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
 | Filename:       Consumer.h               | Consumer                        |
 | Author  : Francis Maes                   |                                 |
 | Started : 22/03/2010 15:50               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/
#ifndef LBCPP_CONSUMER_H_
# define LBCPP_CONSUMER_H_

# include "Function.h"

namespace lbcpp
{
  
class Consumer : public Function
{
public:
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return nilType();}
  
  virtual void consume(const Variable& variable) = 0;
  
  void consumeStream(StreamPtr stream, size_t maximumCount = 0);
  void consumeContainer(ContainerPtr container);

protected:
  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
    {const_cast<Consumer* >(this)->consume(input); return Variable();}
};

class TextPrinter : public Consumer
{
public:
  TextPrinter(OutputStream* newOutputStream);
  TextPrinter(const File& file);
  
  virtual ~TextPrinter()
    {if (ostr) delete ostr;}
  
protected:
  OutputStream* ostr;   
  
  void printNewLine()
  {if (ostr) (*ostr) << "\n";}
  
  void print(const String& str, bool addNewLine = false)
  {
    if (ostr)
    {
      (*ostr) << (const char* )str;
      if (addNewLine)
        (*ostr) << "\n";
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONSUMER_H_
