/*-----------------------------------------.---------------------------------.
| Filename: FileObjectComponent.h          | File Object Component           |
| Author  : Francis Maes                   |                                 |
| Started : 17/06/2010 11:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_FILE_OBJECT_H_
# define EXPLORER_COMPONENTS_FILE_OBJECT_H_

# include "../Utilities/ComponentWithPreferedSize.h"

namespace lbcpp
{

class FileObject : public Object
{
public:
  FileObject(const File& file)
    : file(file)
  {
    if (file.isDirectory())
    {
      file.findChildFiles(subDirectories, File::findDirectories, false);
      file.findChildFiles(subFiles, File::findFiles, false);
    }
  }

  FileObject() {}

  virtual String getName() const
    {return file.getFileName();}

  virtual String toString() const
    {return file.loadFileAsString();}

  File getFile() const
    {return file;}

  enum Type
  {
    textFile,
    binaryFile,
    classFile,
    directory,
    classDirectory,
    nonexistent,
    xmlFile
  };

  Type getType() const
  {
    if (!file.exists())
      return nonexistent;
    if (file.isDirectory())
      return file.getChildFile(T(".classFile")).exists() ? classDirectory : directory;
    
    if (file.getSize() > 100 * 1024 * 1024) // do not open files that do more than 100 Mb
      return binaryFile;

    const juce::MemoryBlock& data = getData();
    
    int indexOfZero = -1;
    String beforeZero;
    for (int i = 0; i < juce::jmin(256, data.getSize()); ++i)
      if (data[i] == 0)
      {
        indexOfZero = i;
        break;
      }
      else
        beforeZero += data[i];

    if (indexOfZero < 0)
    {
      static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
      if ((size_t)data.getSize() > strlen(xmlBegin) && memcmp(data.getData(), xmlBegin, strlen(xmlBegin)) == 0)
        return xmlFile;
      return textFile;
    }
    if (Class::doClassNameExists(beforeZero))
      return classFile;

    return binaryFile;
  }

  bool isDirectory() const
    {return file.isDirectory();}

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const
  {
    subObjects.clear();
    subObjects.reserve(subDirectories.size() + subFiles.size());
    for (int i = 0; i < subDirectories.size(); ++i)
      addChildrenObject(*subDirectories[i], subObjects);
    for (int i = 0; i < subFiles.size(); ++i)
      addChildrenObject(*subFiles[i], subObjects);
  }

  static void addChildrenObject(const File& subFile, std::vector< std::pair<String, ObjectPtr> >& res)
    {res.push_back(std::make_pair(subFile.getFileName(), ObjectPtr(new FileObject(subFile))));}

  const juce::MemoryBlock& getData() const
  {
    jassert(!file.isDirectory());
    if (data.getSize() == 0)
      file.loadFileAsData(const_cast<FileObject* >(this)->data);
    return data;
  }

private:
  File file;
  juce::OwnedArray<File> subDirectories;
  juce::OwnedArray<File> subFiles;
  juce::MemoryBlock data;
};

typedef ReferenceCountedObjectPtr<FileObject> FileObjectPtr;

class HexadecimalMemoryViewer : public Component, public ComponentWithPreferedSize
{
public:
  HexadecimalMemoryViewer(const juce::MemoryBlock& data)
    : data(data) {}

  enum
  {
    letterWidth = 25, letterHeight = 14,
    leftMargin = 80, rightMargin = 10,
    verticalMargin = 10,
  };

  int getSizePerLine(int availableWidth) const
  {
    int res = (availableWidth - leftMargin - rightMargin) / letterWidth;
    return (res / 10) * 10;
  }

  int getNumLines(int sizePerLine, juce::int64 totalSize) const
    {return (int)ceil((double)totalSize / (double)sizePerLine);}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return 2 * verticalMargin + getNumLines(getSizePerLine(availableWidth), data.getSize());}

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);

    int sizePerLine = getSizePerLine(getWidth());
    int numLines = getNumLines(sizePerLine, data.getSize());
    int x = leftMargin;
    int y = verticalMargin;
    int xEnd = x + sizePerLine * letterWidth;

    g.setColour(Colours::grey);
    g.setFont(8);
    for (int i = 0; i < numLines; ++i)
      g.drawText(String(i * sizePerLine), 5, y, leftMargin - 10, letterHeight, Justification::centredRight, false);
    
    g.setColour(Colours::black);
    g.setFont(12);
    unsigned char* bytes = (unsigned char* )data.getData();
    for (int i = 0; i < numLines; ++i)
    {
      g.drawLine((float)x, (float)y, (float)xEnd, (float)y);
      int offset = i * sizePerLine;
      int size = juce::jmin(sizePerLine, data.getSize() - offset);
      int yEnd = y + letterHeight;
      int xc = x;
      for (int j = 0; j < size; ++j)
      {
        g.drawLine((float)xc, (float)y, (float)xc, (float)yEnd);
        g.drawText(String::toHexString(bytes + offset + j, 1), xc, y, letterWidth, letterHeight, Justification::centred, false);
        xc += letterWidth;
      }
      g.drawLine((float)xc, (float)y, (float)xc, (float)yEnd);
      y = yEnd;
    }
    g.drawLine((float)x, (float)y, (float)xEnd, (float)y);
  }

protected:
  const juce::MemoryBlock& data;
};

class HexadecimalFileObjectComponent : public ViewportComponent
{
public:
  HexadecimalFileObjectComponent(FileObjectPtr file, const String& name)
    : ViewportComponent(new HexadecimalMemoryViewer(file->getData()), true, false)
    {}
};


}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_BROWSER_H_

