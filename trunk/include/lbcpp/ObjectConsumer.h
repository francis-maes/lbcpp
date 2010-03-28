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
| Filename: ObjectConsumer.h               | Object Consumer                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2010 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_OBJECT_CONSUMER_H_
# define LBCPP_OBJECT_CONSUMER_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class ObjectConsumer : public Object
{
public:
  virtual void consume(ObjectPtr object) = 0;
  
  void consume(ObjectStreamPtr stream, size_t maximumCount = 0);
  void consume(ObjectContainerPtr container);
};

class TextObjectPrinter : public ObjectConsumer
{
public:
  TextObjectPrinter(OutputStream* newOutputStream);
  TextObjectPrinter(const File& file);

  virtual ~TextObjectPrinter()
    {if (ostr) delete ostr;}
    
protected:
  OutputStream* ostr;   

  void printNewLine()
    {if (ostr) (*ostr) << "\n";}
    
  void print(const String& str)
    {if (ostr) (*ostr) << (const char* )str;}    
};

class LearningDataObjectPrinter : public TextObjectPrinter
{
protected:
  LearningDataObjectPrinter(const File& file)
    : TextObjectPrinter(file) {}

  LearningDataObjectPrinter(OutputStream* newOutputStream)
    : TextObjectPrinter(newOutputStream) {}

  void printFeatureList(FeatureGeneratorPtr features);
};

extern ObjectConsumerPtr vectorObjectContainerFiller(VectorObjectContainerPtr container);
extern ObjectConsumerPtr classificationExamplesPrinter(const File& file, StringDictionaryPtr labels);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CONSUMER_H_
