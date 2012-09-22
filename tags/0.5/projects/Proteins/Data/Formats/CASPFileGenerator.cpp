/*-----------------------------------------.---------------------------------.
| Filename: CASPFileGenerator.cpp          | CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "CASPFileGenerator.h"
#include "PDBFileGenerator.h"
using namespace lbcpp;

/*
** CASPFileGenerator
*/
CASPFileGenerator::CASPFileGenerator(ExecutionContext& context, const File& file, const String& method)
  : TextPrinter(context, file), method(method) {}

void CASPFileGenerator::consume(ExecutionContext& context, const Variable& variable)
{
  const ProteinPtr& protein = variable.getObjectAndCast<Protein>(context);
  jassert(protein);

  printRecord(T("PFRMAT"), getFormatSpecificationCode());
  printRecord(T("TARGET"), protein->getName());
  printRecord(T("AUTHOR"), T("ULg-GIGA")); // T("9344-7768-6843"));
  printMultiLineRecord(T("REMARK"), T("This is a prediction of server ULg-GIGA made at ") + Time::getCurrentTime().toString(true, true, true, true));
  printMultiLineRecord(T("METHOD"), method);
  printRecord(T("MODEL"), T("1"));
  printPredictionData(protein);
  print("END", true);
}

void CASPFileGenerator::printRecord(const String& keyword, const String& data)
{
  String key = keyword;
  while (key.length() < 6)
    key += T(" ");
  print(key + T(" ") + data + T("\n"));
}

void CASPFileGenerator::printMultiLineRecord(const String& keyword, const String& text)
{
  int b = 0;
  int n = text.indexOfChar(0, '\n');
  while (n >= 0)
  {
    printMultiLineRecordBase(keyword, text.substring(b, n));
    b = n + 1;
    n = text.indexOfChar(b, '\n');
  }
  if (b < text.length())
    printMultiLineRecordBase(keyword, text.substring(b));
}

void CASPFileGenerator::printMultiLineRecordBase(const String& keyword, const String& text)
{
  if (text.isEmpty())
    return;

  String line = keyword;
  while (line.length() < 6)
    line += T(" ");
  line += T(" ");

  int remainingCharacters = maxColumns - line.length();
  if (text.length() <= remainingCharacters)
    print(line + text, true);
  else
  {
    int i;
    for (i = remainingCharacters - 1; i >= 0; --i)
      if (text[i] == ' ' || text[i] == '\t')
        break;
    if (i < 0)
    {
      // could not find a space to break the line, force-break
      print(line + text.substring(0, remainingCharacters - 1) + T("-"), true);
      printMultiLineRecordBase(keyword, text.substring(remainingCharacters - 1));
    }
    else
    {
      print(line + text.substring(0, i), true);
      printMultiLineRecordBase(keyword, text.substring(i + 1));
    }
  }
}

#include "OrderDisorderRegionCASPFileGenerator.h"
#include "ResidueResidueDistanceCASPFileGenerator.h"
#include "TertiaryStructureCASPFileGenerator.h"

ConsumerPtr lbcpp::caspOrderDisorderRegionFileGenerator(ExecutionContext& context, const File& file, const String& method)
  {return new OrderDisorderRegionCASPFileGenerator(context, file, method);}

ConsumerPtr lbcpp::caspResidueResidueDistanceFileGenerator(ExecutionContext& context, const File& file, const String& method)
  {return new ResidueResidueDistanceCASPFileGenerator(context, file, method);}

ConsumerPtr lbcpp::caspTertiaryStructureFileGenerator(ExecutionContext& context, const File& file, const String& method)
  {return new TertiaryStructureCASPFileGenerator(context, file, method);}
