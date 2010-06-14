/*-----------------------------------------.---------------------------------.
| Filename: PropertyListDisplayComponent.h | Displays a list of properties   |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 16:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef EXPLORER_UTILITIES_PROPERTY_LIST_DISPLAY_COMPONENT_H_
# define EXPLORER_UTILITIES_PROPERTY_LIST_DISPLAY_COMPONENT_H_

# include "../Components/common.h"

namespace lbcpp
{

class PropertyListDisplayComponent : public Component
{
public:
  PropertyListDisplayComponent(int namesWidth = 120)
    : namesWidth(namesWidth) {}

  void clearProperties()
    {properties.clear(); repaint();}

  void addProperty(const String& name, const String& value)
    {properties.push_back(std::make_pair(name, value)); repaint();}

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::antiquewhite);
    if (!properties.size())
      return;
    int baseHeight = getHeight() / properties.size();
    for (size_t i = 0; i < properties.size(); ++i)
    {
      int h1 = (int)(i * baseHeight);
      Font nameFont(12, Font::bold);
      g.setFont(nameFont);
      g.drawText(properties[i].first, 0, h1, namesWidth, baseHeight, Justification::centredLeft, true);
      
      Font valueFont(12, Font::plain);
      g.setFont(valueFont);
      g.drawText(properties[i].second, namesWidth, h1, getWidth() - namesWidth, baseHeight, Justification::centredLeft, true);
    }
  }
  
  juce_UseDebuggingNewOperator

private:
  int namesWidth;
  std::vector< std::pair<String, String> > properties;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_PROPERTY_LIST_DISPLAY_COMPONENT_H_
