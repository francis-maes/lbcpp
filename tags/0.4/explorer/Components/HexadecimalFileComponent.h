/*-----------------------------------------.---------------------------------.
| Filename: HexadecimalFileComponent.h     | A component that displays an    |
| Author  : Francis Maes                   |  hexadecimal file               |
| Started : 20/08/2010 11:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_HEXADECIMAL_FILE_H_
# define EXPLORER_COMPONENTS_HEXADECIMAL_FILE_H_

# include "common.h"
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>

namespace lbcpp
{

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

class HexadecimalFileComponent : public ViewportComponent
{
public:
  HexadecimalFileComponent(const Variable& file, const String& name)
    : ViewportComponent(NULL, true, false)
  {
    file.getFile().loadFileAsData(data);
    setViewedComponent(new HexadecimalMemoryViewer(data));
  }

protected:
  juce::MemoryBlock data;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_FILE_TYPE_H_
