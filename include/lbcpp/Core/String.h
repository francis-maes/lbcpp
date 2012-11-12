/*-----------------------------------------.---------------------------------.
| Filename: String.h                       | String                          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 17:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_STRING_H_
# define LBCPP_CORE_STRING_H_

# include "Object.h"

namespace lbcpp
{

class NewString : public Object
{
public:
  NewString(ClassPtr thisClass, const juce::String& value = juce::String::empty)
    : Object(thisClass), value(value) {}
  NewString(const juce::String& value = juce::String::empty)
    : value(value) {}

  void set(const juce::String& value)
    {this->value = value;}

  const juce::String& get() const
    {return value;}
  
  static juce::String get(const ObjectPtr& object)
    {return object.staticCast<NewString>()->get();}

  virtual juce::String toShortString() const;

  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  virtual bool loadFromString(ExecutionContext& context, const String& str);
  virtual String toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  juce::String value;
};

extern ClassPtr newStringClass;

class File : public NewString
{
public:
  File(ClassPtr thisClass, const juce::File& file)
    : NewString(thisClass, file.getFullPathName()) {}
  File(const juce::File& file)
    : NewString(file.getFullPathName()) {}
  File() {}

  static FilePtr create(const juce::File& file);
  static juce::File get(const ObjectPtr& object)
    {return object.staticCast<File>()->get();}

  void set(const juce::File& value)
    {this->value = value.getFullPathName();}

  juce::File get() const
    {return juce::File(value);}

  virtual juce::String toShortString() const;

  virtual juce::String toString() const;
  virtual bool loadFromString(ExecutionContext& context, const String& str);

  virtual void saveToXml(XmlExporter& exporter) const;
};

extern ClassPtr fileClass;

class Directory : public File
{
public:
  Directory(const juce::File& file)
    : File(file.getFullPathName()) {}
  Directory() {}
};

extern ClassPtr directoryClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_
