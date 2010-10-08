/*-----------------------------------------.---------------------------------.
| Filename: MultiSequenceComponent.h       | Components to display multiple  |
| Author  : Francis Maes                   |  aligned sequences              |
| Started : 14/06/2010 18:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_
# define EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

# include "../Components/common.h"
# include "../Utilities/ComponentWithPreferedSize.h"
# include "../Utilities/VariableSelector.h"
# include "../../projects/Proteins/Data/AminoAcid.h"

namespace lbcpp
{

class MultiSequenceComponent : public Component, public ComponentWithPreferedSize, public VariableSelector
{
public:
  MultiSequenceComponent() : numSequences(0), longestSequence(0) {}
  
  void clearSequences()
    {sequences.clear();}

  void addSequenceGroup(const String& friendlyName, const std::vector< std::pair<String, ContainerPtr> >& sequences)
  {
    this->sequences.push_back(std::make_pair(friendlyName, sequences));
    numSequences += sequences.size();
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      size_t length = sequences[i].second->getNumElements();
      if (length > longestSequence)
        longestSequence = length;
    }
  }

  bool hitTest(int x, int y, String& sequenceName, String& versionName, int& position)
  {
    position = -1;
    if (lines.empty() || y < lines[0].y1)
      return false;

    for (size_t i = 0; i < lines.size(); ++i)
      if (y >= lines[i].y1 && y < lines[i].y1 + elementHeight)
      {
        LineInfo info = lines[i];
        size_t length = info.end - info.begin;
        if (x < info.x1 || x >= (int)(info.x1 + length * elementWidth))
          return false;
        int positionInLine = (x - info.x1) / elementWidth;
        position = (int)info.begin + positionInLine;
        sequenceName = info.sequenceName;
        versionName = info.versionName;
        return true;
      }
    return false;
  }

  virtual void mouseUp(const MouseEvent& e)
  {
    String sequenceName, versionName;
    int position;
    if (hitTest(e.getMouseDownX(), e.getMouseDownY(), sequenceName, versionName, position))
      sendSelectionChanged(Variable::pair(Variable::pair(sequenceName, versionName), position));
    else
      sendSelectionChanged(std::vector<Variable>());
  }

  enum
  {
    marginLeft = 100,
    marginRight = 5,
    elementWidth = 20,
    elementHeight = 23,
    marginVertical = 20,
  };

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int numElementsPerLine = (availableWidth - marginLeft - marginRight) / elementWidth;
    if (numElementsPerLine < 1)
      return availableHeight;
    int numParts = (int)ceil(longestSequence / (double)numElementsPerLine);
    return numParts * (numSequences * elementHeight + marginVertical);
  }

  virtual void paint(Graphics& g)
  {
    lines.clear();
    g.fillAll(Colours::white);
    int numElementsPerLine = (getWidth() - marginLeft - marginRight) / elementWidth;
    int numParts = (int)ceil(longestSequence / (double)numElementsPerLine);

    int y = marginVertical / 2;
    size_t begin = 0;
    for (int i = 0; i < numParts; ++i)
    {
      size_t end = begin + numElementsPerLine;
      if (end > longestSequence)
        end = longestSequence;
      paintSequencesInterval(g, begin, end, marginLeft, y);
      begin = end;
      y += elementHeight * numSequences + marginVertical;
    }
  }

  void setSelection(int position, const String& sequence, const String& version)
  {
    selectedPosition = position;
    selectedSequence = sequence;
    selectedVersion = version;
    repaint();
  }

  void setNoSelection()
    {selectedPosition = -1; selectedSequence = selectedVersion = String::empty; repaint();}

private:
  int selectedPosition;
  String selectedSequence, selectedVersion;

  std::vector< std::pair<String, std::vector< std::pair<String, ContainerPtr> > > > sequences;
  size_t numSequences, longestSequence;

  struct LineInfo
  {
    String sequenceName, versionName;
    size_t begin, end;
    int x1, y1;
  };
  std::vector<LineInfo> lines;

  void paintSequencesInterval(Graphics& g, size_t begin, size_t end, int x1, int y1)
  {
    g.setFont(10);
    g.setColour(Colours::grey);
    for (size_t i = begin; i < end; ++i)
      if (i == begin || i == end - 1 || ((i + 1) % 5) == 0)
        g.drawText(String((int)(i + 1)), x1 + (i - begin) * elementWidth, y1 - 10, elementWidth, 10, Justification::centred, true);

    bool hasMultiSequences = false;
    for (size_t i = 0; i < sequences.size(); ++i)
      if (sequences[i].second.size() > 1)
      {
        hasMultiSequences = true;
        break;
      }
    
    int y = y1;
    for (size_t i = 0; i < sequences.size(); ++i)
    {
      std::vector< std::pair<String, ContainerPtr> >& seq = sequences[i].second;
      for (size_t j = 0; j < seq.size(); ++j)
      {
        g.setFont(9);
        g.setColour(Colours::black);
        if (hasMultiSequences)
        {
          g.drawText(sequences[i].first, 0, y + elementHeight / 2, marginLeft - 5, elementHeight / 2, Justification::centredRight, true);
          g.setColour(Colours::grey);
          g.drawText(seq[j].first, 0, y, marginLeft - 5, elementHeight / 2, Justification::centredRight, true);
        }
        else
          g.drawText(sequences[i].first, 0, y, marginLeft - 5, elementHeight, Justification::centredRight, true);
          
        LineInfo info;
        info.begin = begin;
        info.end = end;
        info.x1 = x1;
        info.y1 = y;
        info.sequenceName = sequences[i].first;
        info.versionName = seq[j].first;
        lines.push_back(info);
        paintSequenceInterval(g, info, seq[j].first, seq[j].second);
        y += elementHeight;

        if (j < seq.size() - 1)
          paintInterSequenceInterval(g, begin, end, x1, y, seq[j].second, seq[j + 1].second); 
      }
    }
    if (selectedPosition >= (int)begin && selectedPosition < (int)end)
    {
      g.setColour(Colours::lightblue.withAlpha(0.2f));
      int pos = selectedPosition - (int)begin;
      g.fillRect(x1 + pos * elementWidth, y1, elementWidth, y - y1);
    }
  }

  void paintSequenceInterval(Graphics& g, const LineInfo& info, const String& versionName, ContainerPtr sequence)
  {
    int y2 = info.y1 + elementHeight;
    
    g.setColour(Colours::black);
    size_t length = info.end - info.begin;
    g.drawLine((float)info.x1, (float)info.y1, (float)(info.x1 + length * elementWidth), (float)info.y1);
    g.drawLine((float)info.x1, (float)y2, (float)(info.x1 + length * elementWidth), (float)y2);
    
    int x = info.x1;
    for (size_t i = info.begin; i < info.end; ++i)
    {
      g.setColour(Colours::black);
      Variable variable1 = i > 0 ? sequence->getElement(i - 1) : Variable();
      Variable variable2 = sequence->getElement(i);
      
      if (variable1 && variable2 && variable1 == variable2)
      {
        g.setColour(Colours::lightgrey);
        g.drawLine((float)x, (float)(info.y1 + 1), (float)x, (float)y2);
        g.setColour(Colours::black);
      }
      else
        g.drawLine((float)x, (float)(info.y1 + 1), (float)x, (float)y2);
      
      paintSequenceElement(g, x + 1, info.y1 + 1, elementWidth - 1, elementHeight - 1, sequence, i);
      x += elementWidth;
    }
    g.setColour(Colours::black);
    g.drawLine((float)x, (float)info.y1, (float)x, (float)(y2 + 1));
  }

  void paintSequenceElement(Graphics& g, int x, int y, int w, int h, ContainerPtr seq, size_t index)
  {
    VectorPtr sequence = seq.dynamicCast<Vector>();
    if (!sequence)
      return;
    
    TypePtr type = sequence->getElementsType();
    Variable value = sequence->getElement(index);

    if (type->inheritsFrom(enumValueType))
    {
      g.setFont(12.f);
      String res = T("?");
      if (value)
        res[0] = type.dynamicCast<Enumeration>()->getOneLetterCode(value.getInteger());
      g.drawText(res, x, y, w, h, Justification::centred, true);
      return;
    }

    if (type->canBeCastedTo(probabilityType))
    {
      String str = T("?");
      if (sequence->getElement(index))
      {
        g.setColour(Colours::red);
        double realValue = sequence->getElement(index).getDouble();
        double limitedValue = juce::jlimit(0.0, 1.0, realValue);
        g.fillRect(x, y + (int)(h * (1 - limitedValue)), w, (int)(h * limitedValue + 1));
        str = String(realValue, 2);
      }
      g.setColour(Colours::black);
      g.setFont(8);
      g.drawText(str, x, y, w, h, Justification::centred, true);
      return;
    }

    if (type->canBeCastedTo(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration)))
    {
      DiscreteProbabilityDistributionPtr probs = sequence->getElement(index).getObjectAndCast<DiscreteProbabilityDistribution>();
      size_t numVariables = probs->getEnumeration()->getNumElements();
      for (size_t i = 0; i < numVariables; ++i)
      {
        juce::uint8 level = (juce::uint8)(255 * juce::jlimit(0.0, 1.0, 1.0  - probs->getVariable(i).getDouble()));
        g.setColour(Colour(255, level, level));
        int y1 = y + i * h / numVariables;
        int y2 = y + (i + 1) * h / numVariables;
        g.fillRect(x, y1, w, y2 - y1);
      }
      return;
    }
    std::cout << "No paint action implemented for element of type: " << type->toString() << std::endl;
    jassert(false);
  }

  void paintInterSequenceInterval(Graphics& g, size_t begin, size_t end, int x1, int y, ContainerPtr sequence1, ContainerPtr sequence2)
  {
    for (size_t i = begin; i < end; ++i)
    {
      Variable v1 = sequence1->getElement(i);
      Variable v2 = sequence2->getElement(i);
      if (v1 && v2)
      {
        float x = (float)(x1 + (i - begin) * elementWidth + elementWidth / 2);
        if (v1 == v2)
        {
          g.setColour(Colours::lightgreen);
          enum {lineSize = 3};
          g.drawLine(x, (float)(y - lineSize), x, (float)(y + lineSize + 1), 2);
        }
        else
        {
          g.setColour(Colours::red);
          enum {crossSize = 4};
          g.drawLine(x - crossSize, (float)(y - crossSize), x + crossSize + 1, (float)(y + crossSize + 1));
          g.drawLine(x - crossSize, (float)(y + crossSize), x + crossSize + 1, (float)(y - crossSize - 1));
        }
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

