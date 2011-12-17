/*-----------------------------------------.---------------------------------.
| Filename: IndexSet.cpp                   | Index Set                       |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 13:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Data/IndexSet.h>
using namespace lbcpp;

IndexSet::IndexSet(size_t begin, size_t end) : totalSize(0)
{
  addFilledChunk(begin, end);
}

IndexSet::IndexSet() : totalSize(0)
{
}

void IndexSet::append(size_t index, double minimumSparsity)
{
  if (chunks.size() == 0)
    chunks.push_back(std::vector<size_t>(1, index));
  else
  {
    Chunk& lastChunk = chunks.back();
    jassert(lastChunk.size() && lastChunk.back() < index);
    double newSparsity = (lastChunk.size() + 1) / (double)(index - lastChunk.front());
    if (newSparsity < minimumSparsity)
      // create a new chunk
      chunks.push_back(std::vector<size_t>(1, index));
    else
      // append to last chunk
      lastChunk.push_back(index);
  }    
  ++totalSize;
}

void IndexSet::addFilledChunk(size_t begin, size_t end)
{
  jassert(begin <= end);
  if (begin == end)
    return;

  Chunk newChunk(end - begin);
  fillChunk(newChunk, begin, end);
  insertChunk(newChunk);
}

size_t IndexSet::getIndexOfFirstChunkAfter(size_t position) const
{
  for (size_t i = 0; i < chunks.size(); ++i)
    if (chunks[i].front() >= position)
      return i;
  return chunks.size();
}

String IndexSet::IntersectionRegion::toString() const
{
  String res = T("[");
  res += String((int)begin) + T(", ") + String((int)end) + T("[");
  if (hasFirst())
    res += T(" first = ") + String(firstChunkNumber) + T(":") + String(firstIndexInChunk) + T(" (size = ") + String((int)firstCount) + T(")");
  else
    res += T(" no first");
  if (hasSecond())
    res += T(" second = ") + String(secondChunkNumber) + T(":") + String(secondIndexInChunk) + T(" (size = ") + String((int)secondCount) + T(")");
  else
    res += T(" no second");
  return res;
}

size_t IndexSet::getNextIntersection(bool insideChunk, const_iterator it) const
{
  return insideChunk ? chunks[it.chunkNumber].back() + 1 : 
      (it.chunkNumber < chunks.size() ? chunks[it.chunkNumber].front() : (size_t)-1);
}

bool IndexSet::isInsideChunk(size_t position, size_t chunkNumber) const
{
  return chunkNumber < chunks.size() && position >= chunks[chunkNumber].front()
    && position <= chunks[chunkNumber].back();
}

void IndexSet::initializeRegion(size_t begin, size_t end, const_iterator& it, int& resChunkNumber, size_t& resIndexInChunk, size_t& resCount)
{
  jassert(it != this->end());

  resChunkNumber = (int)it.chunkNumber;
  resIndexInChunk = it.indexInChunk;
  jassert(*it >= begin);

  Chunk& chunk = chunks[it.chunkNumber];
  if (chunk.back() < end)
  {
    jassert(*it < end);
    resCount += chunk.size() - it.indexInChunk;
    ++it.chunkNumber;
    it.indexInChunk = 0;
  }
  else
  {
    size_t index;
    for (index = it.indexInChunk; index < chunk.size(); ++index)
      if (chunk[index] >= end)
        break;
    resCount += index - it.indexInChunk;
    jassert(index < chunk.size());
    it.indexInChunk = index;
  }
}

IndexSet::IntersectionRegion IndexSet::makeRegion(const IndexSetPtr& first, const IndexSetPtr& second, size_t begin, size_t end,
                                                  bool& insideFirst, const_iterator& firstIt, bool& insideSecond, const_iterator& secondIt)
{
  jassert(end > begin);
  IntersectionRegion res;
  res.begin = begin;
  res.end = end;

  if (insideFirst)
    first->initializeRegion(begin, end, firstIt, res.firstChunkNumber, res.firstIndexInChunk, res.firstCount);
  insideFirst = first->isInsideChunk(end, firstIt.chunkNumber);

  if (insideSecond)
    second->initializeRegion(begin, end, secondIt, res.secondChunkNumber, res.secondIndexInChunk, res.secondCount);
  insideSecond = second->isInsideChunk(end, secondIt.chunkNumber);

  return res;
}

void IndexSet::getIntersectionRegions(const IndexSetPtr& first, const IndexSetPtr& second, std::vector<IntersectionRegion>& res)
{
  bool insideFirst = first->size() && first->front() == 0;
  const_iterator firstIt = first->begin();

  bool insideSecond = second->size() && second->front() == 0;
  const_iterator secondIt = second->begin();

  size_t begin = 0;
  while (true)
  {
    size_t firstEvent = first->getNextIntersection(insideFirst, firstIt);
    size_t secondEvent = second->getNextIntersection(insideSecond, secondIt);
    jassert(firstEvent > begin && secondEvent > begin);
    size_t end = firstEvent < secondEvent ? firstEvent : secondEvent;
    if (end == (size_t)-1)
      break;

    res.push_back(makeRegion(first, second, begin, end, insideFirst, firstIt, insideSecond, secondIt));
    begin = end;
  }
}

// grow size to newSize; all new samples are taken from source starting from random positions and using contiguous blocks
// this set is assumed to be a subset of the source set
void IndexSet::randomlyExpandUsingSource(ExecutionContext& context, size_t newSize, const IndexSetPtr& source, bool contiguous)
{
  RandomGeneratorPtr random = context.getRandomGenerator();

  jassert(newSize > totalSize);
  jassert(newSize <= source->size());

  // make intersection regions
  std::vector<IntersectionRegion> regions;
  getIntersectionRegions(source, refCountedPointerFromThis(this), regions);

  // select candidate regions
  std::vector<size_t> candidateRegions;
  std::vector<double> candidateProbabilities;
  double Z = 0.0;
  const IntersectionRegion* prevRegion = NULL;
  for (int i = regions.size() - 1; i >= 0; --i)
    if (regions[i].hasFirst())
    {
      prevRegion = &regions[i];
      break;
    }

  for (size_t i = 0; i < regions.size(); ++i)
  {
    const IntersectionRegion& region = regions[i];
    if (region.hasFirst() && region.firstCount)
    {
      if (!region.hasSecond() && (!contiguous || chunks.empty() || (prevRegion && prevRegion->hasSecond())))  // if contiguous, we search for a chunk that is expandable
      {
        candidateRegions.push_back(i); // source contains elements, we do not
        double p = (double)region.firstCount;
        candidateProbabilities.push_back(p);
        Z += p;
      }
      prevRegion = &regions[i];
    }
  }

  // sample region and start offset
  jassert(candidateRegions.size());
  size_t regionIndex = candidateRegions[random->sampleWithProbabilities(candidateProbabilities, Z)];
  size_t startOffset = contiguous ? 0 : random->sampleSize(regions[regionIndex].firstCount);

  while (totalSize < newSize)
  {
    IntersectionRegion& region = regions[regionIndex];
    int targetChunkNumber = (startOffset == 0 && regionIndex > 0 ? regions[regionIndex - 1].secondChunkNumber : -1);
    if (targetChunkNumber >= 0) 
    {
      // expand existing chunk
      Chunk& targetChunk = chunks[(size_t)targetChunkNumber];
      region.secondChunkNumber = targetChunkNumber;
      region.secondIndexInChunk = targetChunk.size();
      expandChunkUsingSource(targetChunk, newSize, region, source);
      region.secondCount = targetChunk.size() - region.secondIndexInChunk;
      ++regionIndex;
    }
    else
    {
      // create new chunk
      Chunk newChunk;
      expandChunkUsingSource(newChunk, newSize, region, source, startOffset);
      size_t index = insertChunk(newChunk);
      for (size_t i = 0; i < regions.size(); ++i)
        if (regions[i].secondChunkNumber >= (int)index)
          ++regions[i].secondChunkNumber;

      size_t sourceCount = region.firstCount;

      // update regions
      IntersectionRegion newRegion(region);
      newRegion.secondChunkNumber = -1;
      newRegion.secondIndexInChunk = 0;
      newRegion.secondCount = 0;

      region.begin = newChunk.front();
      region.end = newChunk.back() + 1;
      region.firstIndexInChunk += startOffset;
      region.firstCount = newChunk.size();
      region.secondChunkNumber = (int)index;
      
      region.secondIndexInChunk = 0;
      region.secondCount = newChunk.size();

      if (startOffset > 0)
      {
        IntersectionRegion prevRegion(newRegion);
        prevRegion.end = newChunk.front();
        prevRegion.firstCount = startOffset;
        regions.insert(regions.begin() + regionIndex, prevRegion);
        ++regionIndex;
      }

      if (startOffset + newChunk.size() < sourceCount)
      {
        IntersectionRegion nextRegion(newRegion);
        nextRegion.begin = newChunk.back() + 1;
        nextRegion.firstIndexInChunk += newChunk.size() + startOffset;
        nextRegion.firstCount = sourceCount - newChunk.size() - startOffset;
        ++regionIndex;
        regions.insert(regions.begin() + regionIndex, nextRegion);
      }
    }
    
    // move to next region
    if (totalSize < newSize)
    {
      //std::vector<IntersectionRegion> dbgRegions;
      //getIntersectionRegions(source, refCountedPointerFromThis(this), dbgRegions);
      //jassert(regions == dbgRegions);

      if (regionIndex == regions.size())
          regionIndex = 0;
      while (!regions[regionIndex].hasFirst() || regions[regionIndex].hasSecond())
      {
        ++regionIndex;
        if (regionIndex == regions.size())
          regionIndex = 0;
      }
      startOffset = 0;
    }
  }
  jassert(totalSize == newSize);
}

#if 0
void IndexSet::randomlyExpandUsingSource(ExecutionContext& context, size_t newSize, const IndexSetPtr& source, bool contiguous)
{
  RandomGeneratorPtr random = context.getRandomGenerator();

  jassert(newSize > totalSize);
  jassert(newSize <= source->size());
  while (totalSize < newSize)
  {
    // compute intersection regions and fill candidate regions
    std::vector<IntersectionRegion> regions;
    getIntersectionRegions(source, refCountedPointerFromThis(this), regions);
    std::vector<size_t> candidateRegions;
    std::vector<double> candidateProbabilities;
    double Z = 0.0;
    for (size_t i = 0; i < regions.size(); ++i)
    {
      const IntersectionRegion& region = regions[i];
      const IntersectionRegion* prevRegion = i > 0 ? &regions[i - 1] : (regions.size() ? &regions.back() : NULL);
      if (region.hasFirst() && !region.hasSecond() &&
          (!contiguous || chunks.empty() || (prevRegion && prevRegion->hasSecond())))  // if contiguous, we search for a chunk that is expandable
      {
        candidateRegions.push_back(i); // source contains elements, we do not
        candidateProbabilities.push_back((double)region.firstCount);
        Z += (double)region.firstCount;
      }
    }
    jassert(candidateRegions.size());
    /*
    if (contiguous && candidateRegions.empty())
    {
      // no chunks yet or everything is filled except the beginning
      // -SOURCE SOURCE SOURCE-           -SOURCE SOURCE SOURCE-
      // -empty empty empty empty-        -empty filled filled filled-
      for (size_t i = 0; i < regions.size(); ++i)
      {
        const IntersectionRegion& region = regions[i];
        if (region.hasFirst())
        {
          jassert(!region.hasSecond());
          candidateRegions.push_back(i);
          break;
        }
      }
    }*/

    // sample candidate region
    size_t regionIndex = candidateRegions[random->sampleWithProbabilities(candidateProbabilities, Z)];
    const IntersectionRegion& region = regions[regionIndex];

    // even with "contiguous", the target region may be empty if we insert at beginning
    int targetChunkNumber = contiguous && regionIndex > 0 ? regions[regionIndex - 1].secondChunkNumber : -1;
    if (targetChunkNumber >= 0) 
    {
      // expand existing chunk
      Chunk& targetChunk = chunks[(size_t)targetChunkNumber];
      expandChunkUsingSource(targetChunk, newSize, region, source);
    }
    else
    {
      // create new chunk
      size_t startOffset = contiguous ? 0 : random->sampleSize(region.firstCount);
      Chunk newChunk;
      expandChunkUsingSource(newChunk, newSize, region, source, startOffset);
      insertChunk(newChunk);
    }

    contiguous = true;
  }
}
#endif // 0

void IndexSet::fillChunk(Chunk& chunk, size_t begin, size_t end)
{
  jassert(chunk.size() == end - begin);
  for (size_t i = begin; i < end; ++i)
    chunk[i - begin] = i;
  totalSize += end - begin;
}

// the newChunk should not overlap with existing chunks
size_t IndexSet::insertChunk(const Chunk& newChunk)
{
  size_t begin = newChunk.front();
  size_t end = newChunk.back() + 1;

  int lastChunkBefore = (int)getIndexOfFirstChunkAfter(begin) - 1;
  jassert(lastChunkBefore == -1 || chunks[lastChunkBefore].back() < begin);
  int firstChunkAfter = (int)getIndexOfFirstChunkAfter(end);
  jassert(firstChunkAfter == (int)chunks.size() || chunks[firstChunkAfter].front() >= end);
  jassert(lastChunkBefore == firstChunkAfter - 1);
  chunks.insert(chunks.begin() + firstChunkAfter, newChunk);
  return (size_t)firstChunkAfter;
}

void IndexSet::expandChunkUsingSource(Chunk& targetChunk, size_t targetSize, const IntersectionRegion& region, const IndexSetPtr& source, size_t sourceOffset)
{
  jassert(region.hasFirst());
  const Chunk& sourceChunk = source->chunks[region.firstChunkNumber];
  size_t sourceIndex = region.firstIndexInChunk + sourceOffset;
  jassert(sourceIndex <= sourceChunk.size());

  size_t numberToAdd = targetSize - totalSize;
  if (numberToAdd > region.firstCount - sourceOffset)
    numberToAdd = region.firstCount - sourceOffset;
  if (numberToAdd)
  {
    jassert(sourceIndex + numberToAdd <= sourceChunk.size()); 
    size_t targetIndex = targetChunk.size();
    targetChunk.resize(targetChunk.size() + numberToAdd); 
    memcpy(&targetChunk[targetIndex], &sourceChunk[sourceIndex], sizeof (size_t) * numberToAdd);
    totalSize += numberToAdd;
  }
}
