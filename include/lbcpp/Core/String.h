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
# include "XmlSerialisation.h"
# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class NewString : public Object
{
public:
  NewString(const juce::String& value = juce::String::empty)
    : value(value) {}

  void set(const juce::String& value)
    {this->value = value;}

  const juce::String& get() const
    {return value;}
  
  static juce::String get(const ObjectPtr& object)
    {return object.staticCast<NewString>()->get();}

  virtual juce::String toShortString() const
    {return value;}

  virtual String toString() const
    {return value.quoted();}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const NewStringPtr& other = otherObject.staticCast<NewString>();
    if (value < other->get())
      return -1;
    else if (value > other->get())
      return 1;
    else
      return 0;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<NewString>()->value = value;}
  
  virtual bool loadFromXml(XmlImporter& importer)
  {
    String text = importer.getAllSubText().trim();
    return loadFromString(importer.getContext(), text);
  }

protected:
  juce::String value;
};

extern ClassPtr newStringClass;

class NewFile : public NewString
{
public:
  NewFile(const File& file)
    : NewString(file.getFullPathName()) {}
  NewFile() {}

  static NewFilePtr create(const File& file);

  void set(const juce::File& value)
    {this->value = value.getFullPathName();}

  juce::File get() const
    {return juce::File(value);}

  static juce::File get(const ObjectPtr& object)
    {return object.staticCast<NewFile>()->get();}

  virtual juce::String toShortString() const
    {return get().getFileName();}

  virtual juce::String toString() const
    {return defaultExecutionContext().getFilePath(get());}

  virtual void saveToXml(XmlExporter& exporter) const
    {exporter.addTextElement(toString());}

  virtual bool loadFromString(ExecutionContext& context, const String& str)
  {
    juce::File file = context.getFile(str);
    if (file == File::nonexistent)
    {
      context.errorCallback(T("Could not find file ") + str);
      return false;
    }
    set(file);
    return true;
  }
};

extern ClassPtr newFileClass;

class Directory : public NewFile
{
public:
  Directory(const File& file)
    : NewFile(file.getFullPathName()) {}
  Directory() {}
};

extern ClassPtr directoryClass;

inline NewFilePtr NewFile::create(const File& file)
{
  if (file.isDirectory())
    return new Directory(file);
  else
    return new NewFile(file);
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_DOUBLE_H_
