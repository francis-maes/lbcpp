/*-----------------------------------------.---------------------------------.
| Filename: string.h                       | string                          |
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

class String : public Object
{
public:
  String(ClassPtr thisClass, const string& value = string::empty)
    : Object(thisClass), value(value) {}
  String(const string& value = string::empty)
    : value(value) {}

  void set(const string& value)
    {this->value = value;}

  const string& get() const
    {return value;}
  
  static StringPtr create(ClassPtr type, const string& value);
  static string get(const ObjectPtr& object)
    {return object.staticCast<String>()->get();}

  virtual string toShortString() const;

  virtual int compare(const ObjectPtr& otherObject) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  virtual bool loadFromString(ExecutionContext& context, const string& str);
  virtual string toString() const;

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  string value;
};

extern ClassPtr stringClass;

class File : public String
{
public:
  File(ClassPtr thisClass, const juce::File& file)
    : String(thisClass, file.getFullPathName()) {}
  File(const juce::File& file)
    : String(file.getFullPathName()) {}
  File() {}

  static FilePtr create(const juce::File& file);
  static juce::File get(const ObjectPtr& object)
    {return object.staticCast<File>()->get();}

  void set(const juce::File& value)
    {this->value = value.getFullPathName();}

  juce::File get() const
    {return juce::File(value);}

  virtual string toShortString() const;

  virtual string toString() const;
  virtual bool loadFromString(ExecutionContext& context, const string& str);

  virtual void saveToXml(XmlExporter& exporter) const;
};

extern ClassPtr fileClass;

class Directory : public File
{
public:
  Directory(const juce::File& file)
    : File(file.getFullPathName()) {}
  Directory() {}

  std::vector<FilePtr> findFiles(bool includeFiles = true, bool includeDirectories = true, bool recursively = false, const string& wildCardPattern = "*");
};

extern ClassPtr directoryClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_
