/*-----------------------------------------.---------------------------------.
| Filename: common.h                       | Common include file for         |
| Author  : Francis Maes                   |      object components          |
| Started : 15/06/2009 18:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_COMMON_H_
# define EXPLORER_COMPONENTS_COMMON_H_

# define DONT_SET_USING_JUCE_NAMESPACE
# include "../../juce/juce_amalgamated.h"
# include <lbcpp/lbcpp.h>

using juce::Component;
using juce::TabbedComponent;
using juce::TabbedButtonBar;
using juce::AlertWindow;
using juce::Colours;
using juce::DocumentWindow;
using juce::MenuBarModel;
using juce::JUCEApplication;
using juce::ApplicationCommandManager;
using juce::Viewport;
using juce::TableHeaderComponent;
using juce::TableListBox;
using juce::TableListBoxModel;
using juce::Colour;
using juce::Justification;
using juce::Font;
using juce::Graphics;
using juce::MouseEvent;
using juce::PopupMenu;
using juce::StringArray;
using juce::FileChooser;

namespace lbcpp
{
  extern Component* createComponentForObject(ObjectPtr object, const String& name = String::empty, bool topLevelComponent = false);

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_COMMON_H_
