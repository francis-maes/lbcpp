/*-----------------------------------------.---------------------------------.
| Filename: Utilities.cpp                  | Utilities                       |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Utilities.h>
#include <iostream>
#include <fstream>
#ifndef JUCE_WIN32
# include <cxxabi.h>
#endif // !JUCE_WIN32
using namespace lbcpp;

String lbcpp::getTypeName(const std::type_info& info)
{
#ifdef JUCE_WIN32
  std::string res = info.name();
  size_t n = res.find("::");
  return res.substr(n == std::string::npos ? strlen("class ") : n + 2).c_str();
#else // linux or macos x
  int status;
  char* realname = abi::__cxa_demangle(info.name(), 0, 0, &status);
  String res = realname;
  free(realname);
  return res.startsWith(T("lbcpp::")) ? res.substring(7) : res;
#endif
}
