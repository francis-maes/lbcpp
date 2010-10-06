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
| Filename: TupleType.h                    | Tuple types                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 15:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_TYPE_TUPLE_H_
# define LBCPP_OBJECT_TYPE_TUPLE_H_

# include "BuiltinType.h"

namespace lbcpp
{

class VariableAllocator
{
public:
  VariableAllocator(size_t chunkSize, size_t groupSize)
    : chunkSize(chunkSize), groupSize(groupSize)
  {
  }

  ~VariableAllocator()
  {
   // for (size_t i = 0; i < chunks.size(); ++i)
   //   delete [] chunks[i];
  }

  juce::int64 allocateVariables()
  {
    return (juce::int64)(new Variable[groupSize]());
/*
    ScopedLock _(lock);
    if (freePositions.empty())
    {
      // grow
      size_t newChunkIndex = chunks.size();
      chunks.push_back(new Variable[chunkSize]());
      freePositions.reserve(freePositions.size() + chunkSize / groupSize);
      for (size_t i = 0; i < chunkSize; i += groupSize)
        freePositions.push_back(makeIndex(newChunkIndex, i)); 
    }
      
    juce::int64 res = freePositions.back();
    freePositions.pop_back();
    return res;*/
  }

  void freeVariables(juce::int64 index)
  {
    jassert(index >= 0);
    delete [] (Variable* )index;
    /*ScopedLock _(lock);
    freePositions.push_back(index);
    Variable* data = getData(index);
    for (size_t i = 0; i < groupSize; ++i)
      data[i] = Variable();*/
  }

  const Variable* getData(juce::int64 index) const
  {
    jassert(index >= 0);
    return (const Variable* )index;
    /*
    juce::int64 chunkIndex = index >> 32;
    juce::int64 variableIndex = index & 0xFFFFFFFF;
    jassert(chunkIndex < (juce::int64)chunks.size());
    return chunks[(size_t)chunkIndex] + variableIndex;*/
  }

  Variable* getData(juce::int64 index)
  {
    jassert(index >= 0);
    return (Variable* )index;
    /*
    juce::int64 chunkIndex = index >> 32;
    juce::int64 variableIndex = index & 0xFFFFFFFF;
    jassert(chunkIndex < (juce::int64)chunks.size());
    return chunks[(size_t)chunkIndex] + variableIndex;*/
  }

private:
  size_t chunkSize, groupSize;
/*
  CriticalSection lock;
  std::vector<Variable* > chunks;
  std::vector<juce::int64> freePositions;

  inline juce::int64 makeIndex(size_t chunkIndex, size_t variableIndex) const
    {return ((juce::int64)chunkIndex << 32) | variableIndex;}*/
};

class TupleType : public BuiltinType
{
public:
  TupleType(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType, size_t size, VariableAllocator& allocator)
    : BuiltinType(templateType, templateArguments, baseType), size(size), allocator(allocator) {}
  
  virtual VariableValue getMissingValue() const
    {return VariableValue(-1);}

  virtual void destroy(VariableValue& value) const
  {
    juce::int64 index = value.getInteger();
    if (index >= 0)
      const_cast<TupleType* >(this)->allocator.freeVariables(index);
    value.setInteger(-1);
  }

  virtual String toString(const VariableValue& value) const
  {
    jassert(!isMissingValue(value));
    const Variable* data = getVariables(value);
    String res;
    for (size_t i = 0; i < size; ++i)
    {
      if (i == 0)
        res = T("(");
      else
        res += T(", ");
      res += data[i].toString();
    }
    return res + T(")");
  }

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    Variable* destData = allocateAndGetVariables(dest);
    const Variable* sourceData = getVariables(source);
    for (size_t i = 0; i < size; ++i)
      destData[i] = sourceData[i];
  }
  
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
  {
    const Variable* data1 = getVariables(value1);
    const Variable* data2 = getVariables(value2);
    for (size_t i = 0; i < size; ++i)
    {
      int res = data1[i].compare(data2[i]);
      if (res != 0)
        return res;
    }
    return 0;
  }
  
  virtual size_t getNumElements(const VariableValue& value) const
    {return size;}

  virtual Variable getElement(const VariableValue& value, size_t index) const
  {
    jassert(index < size);
    return getVariables(value)[index];
  }
  
  virtual VariableValue create() const
    {return VariableValue(const_cast<TupleType* >(this)->allocator.allocateVariables());}

  virtual VariableValue createFromXml(XmlImporter& importer) const
  {
    XmlElement* xml = importer.getCurrentElement();
    VariableValue res;
    Variable* data = allocateAndGetVariables(res);
    if (xml->getNumChildElements() != (int)size)
    {
      importer.errorMessage(T("PairType::createFromXml"), T("Invalid number of child elements"));
      return getMissingValue();
    }
    size_t i = 0;
    forEachXmlChildElementWithTagName(*xml, elt, T("element"))
      data[i++] = importer.loadVariable(elt);
    return res;
  }

  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const
  {
    Variable* data = getVariables(value);
    for (size_t i = 0; i < size; ++i)
      exporter.saveElement(i, data[i]);
  }

protected:
  size_t size;
  VariableAllocator& allocator;

  Variable* getVariables(const VariableValue& value) const
  {
    juce::int64 index = value.getInteger();
    return index >= 0 ? const_cast<Variable* >(allocator.getData(index)) : NULL;
  }

  Variable* allocateAndGetVariables(VariableValue& value) const
  {
    juce::int64 index = const_cast<TupleType* >(this)->allocator.allocateVariables();
    value.setInteger(index);
    return const_cast<TupleType* >(this)->allocator.getData(index);
  }
};

class PairType : public TupleType
{
public:
  PairType(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType, VariableAllocator& allocator)
    : TupleType(templateType, templateArguments, baseType, 2, allocator) {jassert(templateArguments.size() == 2);}

  virtual String getElementName(const VariableValue& value, size_t index) const
    {return index ? T("second") : T("first");}

  VariableValue create(const Variable& v1, const Variable& v2) const
  {
    VariableValue res;
    Variable* dest = allocateAndGetVariables(res);
    dest[0] = v1;
    dest[1] = v2;
    return res;
  }

  virtual void copy(VariableValue& dest, const VariableValue& source) const
  {
    Variable* destData = allocateAndGetVariables(dest);
    Variable* sourceData = getVariables(source);
    destData[0] = sourceData[0];
    destData[1] = sourceData[1];
  }
  
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
  {
    Variable* data1 = getVariables(value1);
    Variable* data2 = getVariables(value2);
    int res = data1[0].compare(data2[0]);
    return res ? res : data1[1].compare(data2[1]);
  }

  virtual ObjectPtr clone() const
  {
    jassert(false);
    TypePtr res(new PairType(templateType, templateArguments, baseType, allocator));
    Type::clone(res);
    return res;
  }
};

typedef ReferenceCountedObjectPtr<PairType> PairTypePtr;

class PairTemplateType : public DefaultTemplateType
{
public:
  PairTemplateType(const String& name, const String& baseType)
    : DefaultTemplateType(name, baseType), allocator(10000, 2) {}

  virtual bool initialize(MessageCallback& callback)
  {
    addParameter(T("firstType"));
    addParameter(T("secondType"));
    return DefaultTemplateType::initialize(callback);
  }

  virtual TypePtr instantiate(const std::vector<TypePtr>& arguments, TypePtr baseType, MessageCallback& callback) const
    {jassert(arguments.size() == 2); return new PairType(refCountedPointerFromThis(this), arguments, baseType, const_cast<VariableAllocator& >(allocator));}

private:
  VariableAllocator allocator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_TYPE_TUPLE_H_
