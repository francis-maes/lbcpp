/*-----------------------------------------.---------------------------------.
| Filename: LuapeClassificationSandBox.h   | Luape Classification Sand Box   |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
# define LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Data/Stream.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/Numerical.h> // for lbcpp::convertSupervisionVariableToEnumValue

namespace lbcpp
{

#ifdef JUCE_WIN32
# pragma warning(disable:4996)
#endif // JUCE_WIN32

class JDBDataParser : public TextParser
{
public:
  JDBDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels, bool sparseData = false)
    : TextParser(context, file), features(features), labels(labels), sparseData(sparseData), hasReadDatasetName(false), hasReadAttributes(false) {}
  JDBDataParser() {}

  virtual TypePtr getElementsType() const
    {return pairClass(features, labels);}

  virtual bool parseLine(char* line)
  {
    while (*line == ' ' || *line == '\t')
      ++line;
    if (*line == 0 || *line == ';')
      return true; // skip empty lines and comment lines
    if (!hasReadDatasetName)
    {
      hasReadDatasetName = true;
      return true;
    }
    if (!hasReadAttributes)
    {
      bool res = parseAttributes(line);
      hasReadAttributes = true;
      return res;
    }
    return parseExample(line);
  }

  bool parseAttributes(char* line)
  {
    std::vector< std::pair<String, int> > attributes; // kind: 0 = numerical, 1 = symbolic, 2 = skip

        // f1 NUMERICAL f2 NUMERICAL f3 NUMERICAL f4 NUMERICAL f5 NUMERICAL f6 NUMERICAL f7 NUMERICAL f8 NUMERICAL f9 NUMERICAL f10 NUMERICAL f11 NUMERICAL f12 NUMERICAL f13 NUMERICAL f14 NUMERICAL f15 NUMERICAL f16 NUMERICAL f17 NUMERICAL f18 NUMERICAL f19 NUMERICAL f20 NUMERICAL f21 NUMERICAL f22 NUMERICAL f23 NUMERICAL f24 NUMERICAL f25 NUMERICAL f26 NUMERICAL f27 NUMERICAL f28 NUMERICAL f29 NUMERICAL f30 NUMERICAL f31 NUMERICAL f32 NUMERICAL f33 NUMERICAL f34 NUMERICAL f35 NUMERICAL f36 NUMERICAL f37 NUMERICAL f38 NUMERICAL f39 NUMERICAL f40 NUMERICAL f41 NUMERICAL f42 NUMERICAL f43 NUMERICAL f44 NUMERICAL f45 NUMERICAL f46 NUMERICAL f47 NUMERICAL f48 NUMERICAL f49 NUMERICAL f50 NUMERICAL f51 NUMERICAL f52 NUMERICAL f53 NUMERICAL f54 NUMERICAL f55 NUMERICAL f56 NUMERICAL f57 NUMERICAL f58 NUMERICAL f59 NUMERICAL f60 NUMERICAL f61 NUMERICAL f62 NUMERICAL f63 NUMERICAL f64 NUMERICAL f65 NUMERICAL f66 NUMERICAL f67 NUMERICAL f68 NUMERICAL f69 NUMERICAL f70 NUMERICAL f71 NUMERICAL f72 NUMERICAL f73 NUMERICAL f74 NUMERICAL f75 NUMERICAL f76 NUMERICAL f77 NUMERICAL f78 NUMERICAL f79 NUMERICAL f80 NUMERICAL f81 NUMERICAL f82 NUMERICAL f83 NUMERICAL f84 NUMERICAL f85 NUMERICAL f86 NUMERICAL f87 NUMERICAL f88 NUMERICAL f89 NUMERICAL f90 NUMERICAL f91 NUMERICAL f92 NUMERICAL f93 NUMERICAL f94 NUMERICAL f95 NUMERICAL f96 NUMERICAL f97 NUMERICAL f98 NUMERICAL f99 NUMERICAL f100 NUMERICAL f101 NUMERICAL f102 NUMERICAL f103 NUMERICAL f104 NUMERICAL f105 NUMERICAL f106 NUMERICAL f107 NUMERICAL f108 NUMERICAL f109 NUMERICAL f110 NUMERICAL f111 NUMERICAL f112 NUMERICAL f113 NUMERICAL f114 NUMERICAL f115 NUMERICAL f116 NUMERICAL f117 NUMERICAL f118 NUMERICAL f119 NUMERICAL f120 NUMERICAL f121 NUMERICAL f122 NUMERICAL f123 NUMERICAL f124 NUMERICAL f125 NUMERICAL f126 NUMERICAL f127 NUMERICAL f128 NUMERICAL f129 NUMERICAL f130 NUMERICAL f131 NUMERICAL f132 NUMERICAL f133 NUMERICAL f134 NUMERICAL f135 NUMERICAL f136 NUMERICAL f137 NUMERICAL f138 NUMERICAL f139 NUMERICAL f140 NUMERICAL f141 NUMERICAL f142 NUMERICAL f143 NUMERICAL f144 NUMERICAL f145 NUMERICAL f146 NUMERICAL f147 NUMERICAL f148 NUMERICAL f149 NUMERICAL f150 NUMERICAL f151 NUMERICAL f152 NUMERICAL f153 NUMERICAL f154 NUMERICAL f155 NUMERICAL f156 NUMERICAL f157 NUMERICAL f158 NUMERICAL f159 NUMERICAL f160 NUMERICAL f161 NUMERICAL f162 NUMERICAL f163 NUMERICAL f164 NUMERICAL f165 NUMERICAL f166 NUMERICAL f167 NUMERICAL f168 NUMERICAL f169 NUMERICAL f170 NUMERICAL f171 NUMERICAL f172 NUMERICAL f173 NUMERICAL f174 NUMERICAL f175 NUMERICAL f176 NUMERICAL f177 NUMERICAL f178 NUMERICAL f179 NUMERICAL f180 NUMERICAL f181 NUMERICAL f182 NUMERICAL f183 NUMERICAL f184 NUMERICAL f185 NUMERICAL f186 NUMERICAL f187 NUMERICAL f188 NUMERICAL f189 NUMERICAL f190 NUMERICAL f191 NUMERICAL f192 NUMERICAL f193 NUMERICAL f194 NUMERICAL f195 NUMERICAL f196 NUMERICAL f197 NUMERICAL f198 NUMERICAL f199 NUMERICAL f200 NUMERICAL f201 NUMERICAL f202 NUMERICAL f203 NUMERICAL f204 NUMERICAL f205 NUMERICAL f206 NUMERICAL f207 NUMERICAL f208 NUMERICAL f209 NUMERICAL f210 NUMERICAL f211 NUMERICAL f212 NUMERICAL f213 NUMERICAL f214 NUMERICAL f215 NUMERICAL f216 NUMERICAL f217 NUMERICAL f218 NUMERICAL f219 NUMERICAL f220 NUMERICAL f221 NUMERICAL f222 NUMERICAL f223 NUMERICAL f224 NUMERICAL f225 NUMERICAL f226 NUMERICAL f227 NUMERICAL f228 NUMERICAL f229 NUMERICAL f230 NUMERICAL f231 NUMERICAL f232 NUMERICAL f233 NUMERICAL f234 NUMERICAL f235 NUMERICAL f236 NUMERICAL f237 NUMERICAL f238 NUMERICAL f239 NUMERICAL f240 NUMERICAL f241 NUMERICAL f242 NUMERICAL f243 NUMERICAL f244 NUMERICAL f245 NUMERICAL f246 NUMERICAL f247 NUMERICAL f248 NUMERICAL f249 NUMERICAL f250 NUMERICAL f251 NUMERICAL f252 NUMERICAL f253 NUMERICAL f254 NUMERICAL f255 NUMERICAL f256 NUMERICAL f257 NUMERICAL f258 NUMERICAL f259 NUMERICAL f260 NUMERICAL f261 NUMERICAL f262 NUMERICAL f263 NUMERICAL f264 NUMERICAL f265 NUMERICAL f266 NUMERICAL f267 NUMERICAL f268 NUMERICAL f269 NUMERICAL f270 NUMERICAL f271 NUMERICAL f272 NUMERICAL f273 NUMERICAL f274 NUMERICAL f275 NUMERICAL f276 NUMERICAL f277 NUMERICAL f278 NUMERICAL f279 NUMERICAL f280 NUMERICAL f281 NUMERICAL f282 NUMERICAL f283 NUMERICAL f284 NUMERICAL f285 NUMERICAL f286 NUMERICAL f287 NUMERICAL f288 NUMERICAL f289 NUMERICAL f290 NUMERICAL f291 NUMERICAL f292 NUMERICAL f293 NUMERICAL f294 NUMERICAL f295 NUMERICAL f296 NUMERICAL f297 NUMERICAL f298 NUMERICAL f299 NUMERICAL f300 NUMERICAL f301 NUMERICAL f302 NUMERICAL f303 NUMERICAL f304 NUMERICAL f305 NUMERICAL f306 NUMERICAL f307 NUMERICAL f308 NUMERICAL f309 NUMERICAL f310 NUMERICAL f311 NUMERICAL f312 NUMERICAL f313 NUMERICAL f314 NUMERICAL f315 NUMERICAL f316 NUMERICAL f317 NUMERICAL f318 NUMERICAL f319 NUMERICAL f320 NUMERICAL f321 NUMERICAL f322 NUMERICAL f323 NUMERICAL f324 NUMERICAL f325 NUMERICAL f326 NUMERICAL f327 NUMERICAL f328 NUMERICAL f329 NUMERICAL f330 NUMERICAL f331 NUMERICAL f332 NUMERICAL f333 NUMERICAL f334 NUMERICAL f335 NUMERICAL f336 NUMERICAL f337 NUMERICAL f338 NUMERICAL f339 NUMERICAL f340 NUMERICAL f341 NUMERICAL f342 NUMERICAL f343 NUMERICAL f344 NUMERICAL f345 NUMERICAL f346 NUMERICAL f347 NUMERICAL f348 NUMERICAL f349 NUMERICAL f350 NUMERICAL f351 NUMERICAL f352 NUMERICAL f353 NUMERICAL f354 NUMERICAL f355 NUMERICAL f356 NUMERICAL f357 NUMERICAL f358 NUMERICAL f359 NUMERICAL f360 NUMERICAL f361 NUMERICAL f362 NUMERICAL f363 NUMERICAL f364 NUMERICAL f365 NUMERICAL f366 NUMERICAL f367 NUMERICAL f368 NUMERICAL f369 NUMERICAL f370 NUMERICAL f371 NUMERICAL f372 NUMERICAL f373 NUMERICAL f374 NUMERICAL f375 NUMERICAL f376 NUMERICAL f377 NUMERICAL f378 NUMERICAL f379 NUMERICAL f380 NUMERICAL f381 NUMERICAL f382 NUMERICAL f383 NUMERICAL f384 NUMERICAL f385 NUMERICAL f386 NUMERICAL f387 NUMERICAL f388 NUMERICAL f389 NUMERICAL f390 NUMERICAL f391 NUMERICAL f392 NUMERICAL f393 NUMERICAL f394 NUMERICAL f395 NUMERICAL f396 NUMERICAL f397 NUMERICAL f398 NUMERICAL f399 NUMERICAL f400 NUMERICAL f401 NUMERICAL f402 NUMERICAL f403 NUMERICAL f404 NUMERICAL f405 NUMERICAL f406 NUMERICAL f407 NUMERICAL f408 NUMERICAL f409 NUMERICAL f410 NUMERICAL f411 NUMERICAL f412 NUMERICAL f413 NUMERICAL f414 NUMERICAL f415 NUMERICAL f416 NUMERICAL f417 NUMERICAL f418 NUMERICAL f419 NUMERICAL f420 NUMERICAL f421 NUMERICAL f422 NUMERICAL f423 NUMERICAL f424 NUMERICAL f425 NUMERICAL f426 NUMERICAL f427 NUMERICAL f428 NUMERICAL f429 NUMERICAL f430 NUMERICAL f431 NUMERICAL f432 NUMERICAL f433 NUMERICAL f434 NUMERICAL f435 NUMERICAL f436 NUMERICAL f437 NUMERICAL f438 NUMERICAL f439 NUMERICAL f440 NUMERICAL f441 NUMERICAL f442 NUMERICAL f443 NUMERICAL f444 NUMERICAL f445 NUMERICAL f446 NUMERICAL f447 NUMERICAL f448 NUMERICAL f449 NUMERICAL f450 NUMERICAL f451 NUMERICAL f452 NUMERICAL f453 NUMERICAL f454 NUMERICAL f455 NUMERICAL f456 NUMERICAL f457 NUMERICAL f458 NUMERICAL f459 NUMERICAL f460 NUMERICAL f461 NUMERICAL f462 NUMERICAL f463 NUMERICAL f464 NUMERICAL f465 NUMERICAL f466 NUMERICAL f467 NUMERICAL f468 NUMERICAL f469 NUMERICAL f470 NUMERICAL f471 NUMERICAL f472 NUMERICAL f473 NUMERICAL f474 NUMERICAL f475 NUMERICAL f476 NUMERICAL f477 NUMERICAL f478 NUMERICAL f479 NUMERICAL f480 NUMERICAL f481 NUMERICAL f482 NUMERICAL f483 NUMERICAL f484 NUMERICAL f485 NUMERICAL f486 NUMERICAL f487 NUMERICAL f488 NUMERICAL f489 NUMERICAL f490 NUMERICAL f491 NUMERICAL f492 NUMERICAL f493 NUMERICAL f494 NUMERICAL f495 NUMERICAL f496 NUMERICAL f497 NUMERICAL f498 NUMERICAL f499 NUMERICAL f500 NUMERICAL f501 NUMERICAL f502 NUMERICAL f503 NUMERICAL f504 NUMERICAL f505 NUMERICAL f506 NUMERICAL f507 NUMERICAL f508 NUMERICAL f509 NUMERICAL f510 NUMERICAL f511 NUMERICAL f512 NUMERICAL f513 NUMERICAL f514 NUMERICAL f515 NUMERICAL f516 NUMERICAL f517 NUMERICAL f518 NUMERICAL f519 NUMERICAL f520 NUMERICAL f521 NUMERICAL f522 NUMERICAL f523 NUMERICAL f524 NUMERICAL f525 NUMERICAL f526 NUMERICAL f527 NUMERICAL f528 NUMERICAL f529 NUMERICAL f530 NUMERICAL f531 NUMERICAL f532 NUMERICAL f533 NUMERICAL f534 NUMERICAL f535 NUMERICAL f536 NUMERICAL f537 NUMERICAL f538 NUMERICAL f539 NUMERICAL f540 NUMERICAL f541 NUMERICAL f542 NUMERICAL f543 NUMERICAL f544 NUMERICAL f545 NUMERICAL f546 NUMERICAL f547 NUMERICAL f548 NUMERICAL f549 NUMERICAL f550 NUMERICAL f551 NUMERICAL f552 NUMERICAL f553 NUMERICAL f554 NUMERICAL f555 NUMERICAL f556 NUMERICAL f557 NUMERICAL f558 NUMERICAL f559 NUMERICAL f560 NUMERICAL f561 NUMERICAL f562 NUMERICAL f563 NUMERICAL f564 NUMERICAL f565 NUMERICAL f566 NUMERICAL f567 NUMERICAL f568 NUMERICAL f569 NUMERICAL f570 NUMERICAL f571 NUMERICAL f572 NUMERICAL f573 NUMERICAL f574 NUMERICAL f575 NUMERICAL f576 NUMERICAL f577 NUMERICAL f578 NUMERICAL f579 NUMERICAL f580 NUMERICAL f581 NUMERICAL f582 NUMERICAL f583 NUMERICAL f584 NUMERICAL f585 NUMERICAL f586 NUMERICAL f587 NUMERICAL f588 NUMERICAL f589 NUMERICAL f590 NUMERICAL f591 NUMERICAL f592 NUMERICAL f593 NUMERICAL f594 NUMERICAL f595 NUMERICAL f596 NUMERICAL f597 NUMERICAL f598 NUMERICAL f599 NUMERICAL f600 NUMERICAL f601 NUMERICAL f602 NUMERICAL f603 NUMERICAL f604 NUMERICAL f605 NUMERICAL f606 NUMERICAL f607 NUMERICAL f608 NUMERICAL f609 NUMERICAL f610 NUMERICAL f611 NUMERICAL f612 NUMERICAL f613 NUMERICAL f614 NUMERICAL f615 NUMERICAL f616 NUMERICAL f617 NUMERICAL LETTER-CLASS SYMBOLIC
    bool isFirst = true;
    while (true)
    {
      char* name = strtok(isFirst ? line : NULL, " \t\n");
      char* kind = strtok(NULL, " \t\n");
      if (!name || !kind)
        break;
      isFirst = false;
      int k;
      if (!strcmp(kind, "numerical") || !strcmp(kind, "NUMERICAL"))
        k = 0;
      else if (!strcmp(kind, "symbolic") || !strcmp(kind, "SYMBOLIC"))
        k = 1;
      else if (!strcmp(kind, "name") || !strcmp(kind, "NAME"))
        k = 2;
      else
      {
        context.errorCallback(T("Could not recognize attribute type ") + String(kind).quoted());
        return false;
      }
      attributes.push_back(std::make_pair(name, k));
    }

    // only keep last symbolic attribute
    outputColumnIndex = (size_t)-1;
    for (int i = attributes.size() - 1; i >= 0; --i)
      if (attributes[i].second == 1)
      {
        if (outputColumnIndex == (size_t)-1)
          outputColumnIndex = (size_t)i;
        attributes[i].second = 2;
      }

    columnToVariable.resize(attributes.size(), -1);
    for (size_t i = 0; i < attributes.size(); ++i)
      if (attributes[i].second == 0)
      {
        String name = attributes[i].first;
        int index = features->findOrAddMemberVariable(context, name, doubleType);
        columnToVariable[i] = index;
      }
    return true;
  }

  bool parseExample(char* line)
  {
      // -0.4394 -0.0930 0.1718 0.4620 0.6226 0.4704 0.3578 0.0478 -0.1184 -0.2310 -0.2958 -0.2704 -0.2620 -0.2170 -0.0874 -0.0564 0.0254 0.0958 0.4226 0.6648 0.9184 0.9718 0.9324 0.7070 0.6986 0.7550 0.8816 1.0000 0.9380 0.8450 0.7268 0.5578 -0.4330 -0.1982 0.1270 0.3666 0.4496 0.4258 0.2646 -0.0368 -0.0700 -0.2290 -0.2622 -0.3428 -0.1910 -0.2242 -0.1530 0.0344 0.1080 0.1460 0.3380 0.6726 0.8220 1.0000 0.7912 0.6560 0.6466 0.6916 0.6252 0.6940 0.6986 0.5848 0.4780 0.3334 -0.3872 -0.1290 0.1656 0.4394 0.5228 0.3534 0.1630 -0.0692 -0.1186 -0.1734 -0.2074 -0.4028 -0.2830 -0.3846 -0.2698 -0.1656 0.0170 0.0612 0.1604 0.5958 0.7654 1.0000 0.8070 0.7574 0.7314 0.7184 0.6376 0.6532 0.4472 0.3350 0.3090 0.0144 -0.3698 -0.0616 0.2408 0.5882 0.6806 0.1960 0.0252 0.0644 -0.2212 -0.2830 -0.2942 -0.5994 -0.3838 -0.4566 -0.6834 -0.4034 -0.2352 -0.1456 -0.0896 0.3950 0.5798 0.8768 1.0000 0.9888 0.8712 0.9608 0.9104 1.0000 0.7030 0.6134 0.5350 0.3670 -0.2942 0.0028 0.3080 0.5178 0.6458 0.1090 -0.1880 -0.1444 -0.4414 -0.4468 -0.3896 -0.5286 -0.5832 -0.6268 -0.5504 -0.4904 -0.4578 -0.4060 -0.2344 0.0790 0.3324 0.7030 0.9456 1.0000 0.9810 0.9836 0.9892 0.9100 0.6758 0.5994 0.5150 0.4660 0.0070 0.5840 0.3812 0.4580 0.0700 -0.1084 -0.5944 -0.5140 -0.6468 -0.8252 -0.8006 -0.6538 -0.6994 -0.7518 -0.4336 -0.5174 -0.4896 -0.4930 -0.4896 -0.0034 0.3146 0.8216 0.8426 0.8812 0.8916 1.0000 0.8356 0.5560 0.3042 0.1258 0.0734 -0.0874 -0.4728 0.6218 0.2952 -0.3696 -0.5014 -0.4958 -0.4842 -0.3810 -0.6390 -0.4498 -0.3008 -0.2952 -0.3352 -0.5014 -0.2608 -0.1690 -0.2320 -0.0372 0.2320 0.4900 0.6446 0.4958 0.4040 0.7078 1.0000 0.9140 0.5816 -0.3180 -0.4098 -0.3810 -0.3008 -0.1862 -0.4222 -0.1112 0.1556 0.3778 0.5556 0.5556 0.2888 0.0222 -0.0222 -0.2444 -0.2000 -0.2444 -0.1556 -0.1556 -0.1556 -0.0222 0.0666 0.1556 0.4666 0.7334 0.8666 0.9556 0.8666 0.7334 0.8222 0.9112 0.9112 1.0000 1.0000 0.9112 0.8222 0.7334 -0.5428 -0.1428 0.0858 0.3142 0.4858 0.6000 0.1428 -0.1428 -0.3714 -0.4286 -0.8286 -0.6000 -0.4858 -0.4286 -0.3714 -0.3714 -0.4286 -0.4858 -0.2000 0.1428 0.4858 0.4858 0.7142 0.6000 0.6572 0.8286 0.8286 0.9428 0.9428 1.0000 0.8858 0.7142 -0.2658 0.0632 0.2912 0.2912 0.1392 -0.0380 -0.4936 -0.3924 -0.6962 -0.8482 -0.7974 -0.6962 -0.6202 -0.6456 -0.5444 -0.6456 -0.2912 -0.3924 -0.3164 0.0380 0.1392 0.6202 0.8988 1.0000 0.8228 0.8228 0.5444 0.1898 0.3418 0.1646 0.1140 0.1140 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -0.8950 -0.8650 -0.9100 -0.8500 -0.3850 0.9100 0.9500 0.9050 0.8500 0.8650 0.8900 0.8000 0.5100 0.2600 -0.0900 -0.6350 -0.8900 -0.9050 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -0.8842 -0.8092 -0.8110 -0.1482 -0.4174 0.6814 0.7224 0.8842 0.6286 0.5094 0.4958 0.3152 0.1226 -0.3340 -0.5758 -0.8040 -0.9028 -0.9250 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -0.7600 -0.1500 0.0100 0.2600 0.1432 -0.5250 -0.4218 -0.4718 -0.4384 -0.5518 -0.6484 -0.6186 -0.5216 -0.5294 -0.7448 -0.6278 -0.8534 -0.9268 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -0.3428 -0.4000 -0.5714 -0.3714 1.0000 1.0000 -1.0000 -1.0000 -0.9142 -0.9714 -0.9714 -0.9714 -0.5428 -0.5142 -0.8286 -0.1714 -0.6572 -0.5428 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 -1.0000 1.0000 1.0000 1.0000 1.0000 1.0000 1.0000 1.0000 1.0000 0.9696 0.5646 1.0000 1.0000 1.0000 1.0000 0.9998 0.9946 0.9772 0.9804 1.0000 0.9264 -0.4934 0.4942 -0.4122 0.6270 0.6422 0.6030 0.6934 -0.7500 -0.4824 -0.5464 -0.3738 -0.3482 -0.1948 -0.1694 -0.1694 -0.0990 -0.3418 -0.1374 0.1502 0.0798 0.1758 0.3802 0.7060 0.7252 0.5272 0.5080 0.3802 0.0926 0.2908 0.3610 -0.0734 0.0160 -0.2140 0.1054 0.1694 0.5016 0.7444 0.8274 1.0000 0.7252 -0.5644 -0.5958 -0.3596 -0.1234 -0.2494 -0.2914 -0.1706 -0.1864 -0.3280 0.0604 0.4016 0.4278 0.1864 0.5328 0.6326 0.6482 0.3754 0.3544 0.5854 0.1864 0.8162 0.6536 -0.1286 -0.0132 0.0446 0.2808 0.4804 0.7008 0.7952 0.9212 1.0000 0.5224 -0.6836 -0.5000 -0.1174 -0.1938 -0.4082 -0.3520 -0.3826 -0.4030 -0.1326 0.4234 0.9898 0.9898 0.7806 0.7858 1.0000 0.8674 0.6836 0.5408 0.4796 0.1224 0.8164 0.5816 -0.3572 -0.0408 0.2448 0.1582 0.3316 0.6888 0.8572 0.8622 0.8980 0.5460 -0.8910 -1.0000 1.0000 -1.0000 -1.0000 -1.0000 -1.0000 0.1334 -1.0000 -0.0770 0.0512 0.2564 0.5642 0.4872 0.0770 0.4358 0.7436 0.5128 0.6666 0.6410 0.6154 1.0000 0.8206 0.6410 0.3590 0.6924 0.4358 0.1538 0.4616 0.6154 0.3334 0.3334 0.4102 0.2052 0.3846 0.3590 0.5898 0.3334 0.6410 0.5898 -0.4872 1.
    ObjectPtr inputs = sparseData ? features->createSparseObject() : features->createDenseObject();
    Variable output;
    bool isFirst = true;
    for (size_t i = 0; true; ++i)
    {
      char* token = strtok(isFirst ? line : NULL, " \t\n");
      if (!token)
        break;
      isFirst = false;

      if (i == outputColumnIndex)
        output = Variable(labels->findOrAddElement(context, token), labels);
      else
      {
        int index = columnToVariable[i];
        if (index >= 0)
        {
          double value = strtod(token, NULL);
          inputs->setVariable((size_t)index, value);
        }
      }
    }
    setResult(new Pair(inputs, output));
    return true;
  }

protected:
  DynamicClassPtr features;
  DefaultEnumerationPtr labels;
  bool sparseData;

  bool hasReadDatasetName;
  bool hasReadAttributes;

  size_t outputColumnIndex;
  std::vector<int> columnToVariable;
};

class TestingSetParser : public TextParser
{
public:
  TestingSetParser(ExecutionContext& context, const File& file, ContainerPtr data)
    : TextParser(context, file), data(data) {}
  TestingSetParser() {}

  virtual TypePtr getElementsType() const
  {
    TypePtr exampleType = data->getElementsType();
    return pairClass(containerClass(exampleType), containerClass(exampleType));
  }

  virtual bool parseLine(char* line)
  {
    std::set<size_t> testingIndices;
    bool isFirst = true;
    for (size_t i = 0; true; ++i)
    {
      char* token = strtok(isFirst ? line : NULL, " \t\n");
      if (!token)
        break;
      isFirst = false;
      int index = strtol(token, NULL, 0);
      if (index < 1)
      {
        context.errorCallback(T("Invalid index ") + String(token));
        return false;
      }
      size_t idx = (size_t)(index - 1);
      if (testingIndices.find(idx) != testingIndices.end())
      {
        context.errorCallback(T("Redondant index ") + String(token));
        return false;
      }
      testingIndices.insert(idx);
    }

    size_t n = data->getNumElements();
    TypePtr exampleType = data->getElementsType();
    VectorPtr learningData = vector(exampleType, 0);
    learningData->reserve(n - testingIndices.size());
    VectorPtr testingData = vector(exampleType, 0);
    testingData->reserve(testingIndices.size());

    for (size_t i = 0; i < n; ++i)
      if (testingIndices.find(i) == testingIndices.end())
        learningData->append(data->getElement(i));
      else
        testingData->append(data->getElement(i));

    setResult(new Pair(learningData, testingData));
    return true;
  }

protected:
  ContainerPtr data;
};

class LuapeClassificationSandBox : public WorkUnit
{
public:
  LuapeClassificationSandBox() : maxExamples(0), trainingSize(0), numRuns(0), verbose(false) {}

  typedef LuapeLearnerPtr (LuapeClassificationSandBox::*LearnerConstructor)(LuapeLearnerPtr conditionLearner) const;

  LuapeLearnerPtr singleStumpDiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr singleStumpRealAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return realAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr treeDepth2DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 2);}

  LuapeLearnerPtr treeDepth3DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth3RealAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return realAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth4DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 100, 4);}

  LuapeLearnerPtr treeDepth5DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 100, 5);}

  LuapeLearnerPtr singleTreeLearner(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(false), conditionLearner, 2, 0);}

  LuapeLearnerPtr singleTreeLearnerNormalizedIG(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);}

  LuapeLearnerPtr treeEnsembleLearner(LuapeLearnerPtr conditionLearner) const
    {return ensembleLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeBaggingLearner(LuapeLearnerPtr conditionLearner) const
    {return baggingLearner(singleTreeLearner(conditionLearner), 100);}



  virtual Variable run(ExecutionContext& context)
  {
    // load data
    inputClass = new DynamicClass("inputs");
    labels = new DefaultEnumeration("labels");
    ContainerPtr data = loadData(context, dataFile, inputClass, labels);
    if (!data || !data->getNumElements())
      return false;

    // display data info
    size_t numVariables = inputClass->getNumMemberVariables();
    size_t numExamples = data->getNumElements();
    context.informationCallback(String((int)numExamples) + T(" examples, ") +
                                String((int)numVariables) + T(" variables, ") +
                                String((int)labels->getNumElements()) + T(" labels"));
//    context.informationCallback(String((int)trainingSize) + T(" training examples, ") + String((int)(numExamples - trainingSize)) + T(" testing examples"));

    // make splits
    context.enterScope(T("Splits"));
    if (makeSplits(context, data, splits))
    {
      for (size_t i = 0; i < splits.size(); ++i)
        context.informationCallback(T("Split ") + String((int)i) + T(": train size = ") + String((int)splits[i].first->getNumElements())
                              + T(", test size = ") + String((int)splits[i].second->getNumElements()));
    }
    context.leaveScope(splits.size());
    if (!splits.size())
      return false;

    testLearners(context, &LuapeClassificationSandBox::singleTreeLearner, T("Single tree"));
    testLearners(context, &LuapeClassificationSandBox::singleTreeLearnerNormalizedIG, T("Single tree - normalized IG"));
    testLearners(context, &LuapeClassificationSandBox::treeBaggingLearner, T("Tree Bagging"));
    testLearners(context, &LuapeClassificationSandBox::treeEnsembleLearner, T("Tree Ensemble"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpDiscreteAdaBoostMHLearner, T("Single stump discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpRealAdaBoostMHLearner, T("Single stump real AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth2DiscreteAdaBoostMHLearner, T("Tree depth 2 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3DiscreteAdaBoostMHLearner, T("Tree depth 3 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3RealAdaBoostMHLearner, T("Tree depth 3 real AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth4DiscreteAdaBoostMHLearner, T("Tree depth 4 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth5DiscreteAdaBoostMHLearner, T("Tree depth 5 discrete AdaBoost.MH"));
    return true;
  }

  void testConditionLearner(ExecutionContext& context, LearnerConstructor learnerConstructor, const LuapeLearnerPtr& weakLearner, const String& name, ScalarVariableStatistics& scoreStats) const
  {
    weakLearner->setVerbose(verbose);
    LuapeLearnerPtr learner = (this->*learnerConstructor)(weakLearner);
    learner->setVerbose(verbose);
    scoreStats.push(testConditionLearner(context, learner, name));
  }

  void testLearners(ExecutionContext& context, LearnerConstructor learnerConstructor, const String& name) const
  {
    static const int minExamplesForLaminating = 10;
    size_t numVariables = inputClass->getNumMemberVariables();

    context.enterScope(name);
    
    ScalarVariableStatistics scoreStats;
    LuapeLearnerPtr weakLearner;

    bool isExtraTrees = name.startsWith(T("Extra trees"));

    weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random + randomsplit"), scoreStats);

    weakLearner = exactWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random"), scoreStats);

    weakLearner = exactWeakLearner(inputsNodeBuilder());
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable full"), scoreStats);

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str((int)complexity / 2);
      str += T("-variables ");
      weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random + randomsplit"), scoreStats);

      weakLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random"), scoreStats);

      weakLearner = laminatingWeakLearner(randomSequentialNodeBuilder(numVariables, complexity), (double)numVariables, minExamplesForLaminating);
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("laminating"), scoreStats);
    }
    context.leaveScope(new Pair(scoreStats.getMinimum(), scoreStats.getMean()));
  }

  double testConditionLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name) const
  {
    context.enterScope(name);

    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics(T("error"));

    for (size_t i = 0; i < splits.size(); ++i)
    {
      context.enterScope(T("Split ") + String((int)i));
      context.resultCallback(T("split"), i);
      ScoreObjectPtr score = trainAndTest(context, learner, splits[i].first, splits[i].second);
      jassert(score);
      stats->push(score->getScoreToMinimize());
      context.leaveScope(score->getScoreToMinimize());
      context.progressCallback(new ProgressionState(i+1, splits.size(), T("Splits")));
    }

    context.leaveScope(stats);
    return stats->getMean();
  }

  ScoreObjectPtr trainAndTest(ExecutionContext& context, const LuapeLearnerPtr& learner, const ContainerPtr& trainingData, const ContainerPtr& testingData) const
  {
    LuapeClassifierPtr classifier = createClassifier(inputClass);
    if (!classifier->initialize(context, inputClass, labels))
      return ScoreObjectPtr();
    LuapeBatchLearnerPtr batchLearner = new LuapeBatchLearner(learner);
    classifier->setBatchLearner(batchLearner);
    classifier->setEvaluator(defaultSupervisedEvaluator());
    return classifier->train(context, trainingData, testingData, String::empty, false);
  }

protected:
  friend class LuapeClassificationSandBoxClass;

  File dataFile;
  size_t maxExamples;
  File tsFile;
  size_t trainingSize;
  size_t numRuns;
  bool verbose;


  DynamicClassPtr inputClass;
  DefaultEnumerationPtr labels;
  std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr labels) const
  { 
    static const bool sparseData = true;

    context.enterScope(T("Loading ") + file.getFileName());
    TextParserPtr parser;
    if (file.getFileExtension() == T(".jdb"))
      parser = new JDBDataParser(context, file, inputClass, labels, sparseData);
    else
      parser = classificationARFFDataParser(context, file, inputClass, labels, sparseData);
    ContainerPtr res = parser->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  bool makeSplits(ExecutionContext& context, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
    if (tsFile.existsAsFile())
    {
      TextParserPtr parser = new TestingSetParser(context, tsFile, data);
      ContainerPtr splits = parser->load();
      res.resize(splits->getNumElements());
      for (size_t i = 0; i < res.size(); ++i)
      {
        PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
        res[i] = std::make_pair(split->getFirst().getObjectAndCast<Container>(), split->getSecond().getObjectAndCast<Container>());
      }
    }
    else
    { 
      if (trainingSize >= data->getNumElements())
      {
        context.errorCallback(T("Training size is too big"));
        return false;
      }

      res.resize(numRuns);
      for (size_t i = 0; i < numRuns; ++i)
      {
        ContainerPtr randomized = data->randomize();
        ContainerPtr training = randomized->range(0, trainingSize);
        ContainerPtr testing = randomized->range(trainingSize, randomized->getNumElements());
        res[i] = std::make_pair(training, testing);
      }
    }
    return true;
  }

  LuapeInferencePtr createClassifier(DynamicClassPtr inputClass) const
  {
    LuapeInferencePtr res = new LuapeClassifier();
    size_t n = inputClass->getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      VariableSignaturePtr variable = inputClass->getMemberVariable(i);
      res->addInput(variable->getType(), variable->getName());
    }

    res->addFunction(addDoubleLuapeFunction());
    res->addFunction(subDoubleLuapeFunction());
    res->addFunction(mulDoubleLuapeFunction());
    res->addFunction(divDoubleLuapeFunction());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
