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
# include "../../projects/Proteins/Data/AminoAcid.h"

namespace lbcpp
{

class MultiSequenceComponent : public Component, public ComponentWithPreferedSize
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
      size_t length = sequences[i].second->size();
      if (length > longestSequence)
        longestSequence = length;
    }
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

private:
  std::vector< std::pair<String, std::vector< std::pair<String, ContainerPtr> > > > sequences;
  size_t numSequences, longestSequence;
  
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
          
        paintSequenceInterval(g, begin, end, x1, y, seq[j].first, seq[j].second);
        y += elementHeight;

        if (j < seq.size() - 1)
          paintInterSequenceInterval(g, begin, end, x1, y, seq[j].second, seq[j + 1].second); 
      }
    }
  }

  void paintSequenceInterval(Graphics& g, size_t begin, size_t end, int x1, int y1, const String& versionName, ContainerPtr sequence)
  {
    int y2 = y1 + elementHeight;
    
    g.setColour(Colours::black);
    g.drawLine((float)x1, (float)y1, (float)(x1 + (end - begin) * elementWidth), (float)y1);
    g.drawLine((float)x1, (float)y2, (float)(x1 + (end - begin) * elementWidth), (float)y2);
    
    int x = x1;
    for (size_t i = begin; i < end; ++i)
    {
      g.setColour(Colours::black);
      Variable variable1 = i > 0 ? sequence->getVariable(i - 1) : Variable();
      Variable variable2 = sequence->getVariable(i);
      
      if (variable1 && variable2 && variable1 == variable2)
      {
        g.setColour(Colours::lightgrey);
        g.drawLine((float)x, (float)(y1 + 1), (float)x, (float)y2);
        g.setColour(Colours::black);
      }
      else
        g.drawLine((float)x, (float)(y1 + 1), (float)x, (float)y2);
      
      paintSequenceElement(g, x + 1, y1 + 1, elementWidth - 1, elementHeight - 1, sequence, i);
      x += elementWidth;
    }
    g.setColour(Colours::black);
    g.drawLine((float)x, (float)y1, (float)x, (float)(y2 + 1));
  }

  void paintSequenceElement(Graphics& g, int x, int y, int w, int h, ContainerPtr seq, size_t index)
  {
    VectorPtr sequence = seq.dynamicCast<Vector>();
    if (!sequence)
      return;
    
    TypePtr type = sequence->getElementsType();
    if (type->inheritsFrom(enumerationType()))
    {
      g.setFont(12.f);
      String res = T("?");
      if (sequence->getVariable(index))
        res[0] = type.dynamicCast<Enumeration>()->getOneLetterCode(sequence->getVariable(index).getInteger());
      g.drawText(res, x, y, w, h, Justification::centred, true);
      return;
    }

    if (type->canBeCastedTo(probabilityType()))
    {
      String str = T("?");
      if (sequence->getVariable(index))
      {
        g.setColour(Colours::red);
        double realValue = sequence->getVariable(index).getDouble();
        double limitedValue = juce::jlimit(0.0, 1.0, realValue);
        g.fillRect(x, y + (int)(h * (1 - limitedValue)), w, (int)(h * limitedValue + 1));
        str = String(realValue, 2);
      }
      g.setColour(Colours::black);
      g.setFont(8);
      g.drawText(str, x, y, w, h, Justification::centred, true);
      return;
    }

    if (type->canBeCastedTo(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())))
    {
      DiscreteProbabilityDistributionPtr probs = sequence->getObjectAndCast<DiscreteProbabilityDistribution>(index);
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
      ObjectPtr object1 = sequence1->getObject(i);
      ObjectPtr object2 = sequence2->getObject(i);
      if (object1 && object2)
      {
        float x = (float)(x1 + (i - begin) * elementWidth + elementWidth / 2);
        if (object1->toString() == object2->toString())
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

