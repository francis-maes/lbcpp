/*-----------------------------------------.---------------------------------.
| Filename: Map.cpp                        | Map Containeer base class       |
| Author  : Arnaud Schoofs                 | TODO : not finished !           |
| Started : 10/03/2011 21:41               | (not used currently)            |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Core/Map.h>

using namespace lbcpp;


TypePtr Map::getTemplateKeysParameter(TypePtr type)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Map"));
  jassert(dvType && dvType->getNumTemplateArguments() == 2);
  TypePtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

bool Map::getTemplateKeysParameter(ExecutionContext& context, TypePtr type, TypePtr& res)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Map"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Map"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 2);
  res = dvType->getTemplateArgument(0);
  return true;
}

TypePtr Map::getTemplateValuesParameter(TypePtr type)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Map"));
  jassert(dvType && dvType->getNumTemplateArguments() == 2);
  TypePtr res = dvType->getTemplateArgument(1);
  jassert(res);
  return res;
}

bool Map::getTemplateValuesParameter(ExecutionContext& context, TypePtr type, TypePtr& res)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Map"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Map"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 2);
  res = dvType->getTemplateArgument(1);
  return true;
}