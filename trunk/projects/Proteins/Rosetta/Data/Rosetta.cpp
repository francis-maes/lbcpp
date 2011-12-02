/*-----------------------------------------.---------------------------------.
| Filename: Rosetta.cpp                    | Rosetta source                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:21 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Rosetta.h"
#include "precompiled.h"

namespace lbcpp
{

Rosetta::Rosetta() : context(NULL), lock(new CriticalSection()) {}
Rosetta::Rosetta(ExecutionContext& context) : context(&context), lock(new CriticalSection()) {}
Rosetta::Rosetta(Rosetta& rosetta) : context(rosetta.context), lock(rosetta.lock) {}

void Rosetta::setContext(ExecutionContext& context)
  {this->context = &context;}

void Rosetta::init() {}

void Rosetta::getLock()
  {lock->enter();}
void Rosetta::releaseLock()
  {lock->exit();}

}; /* namespace lbcpp */
