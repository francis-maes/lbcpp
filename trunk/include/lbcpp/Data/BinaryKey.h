/*-----------------------------------------.---------------------------------.
| Filename: BinaryKey.h                    | Binary Key                      |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 11:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_BINARY_KEY_H_
# define LBCPP_DATA_BINARY_KEY_H_

# include "../Core/Object.h"

namespace lbcpp
{

class BinaryKey : public Object
{
public:
  BinaryKey(size_t size)
    : values(size, 0), position(0), bitShift(1) {}
  BinaryKey(const std::vector<unsigned char>& data)
    : values(data), position(data.size()), bitShift(1) {}
  BinaryKey() {}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const std::vector<unsigned char>& ovalues = otherObject.staticCast<BinaryKey>()->values;
    if (ovalues == values)
      return 0;
    else
      return values < ovalues ? -1 : 1;
  }

  void pushBit(bool value)
  {
    jassert(position < values.size());
    if (value)
      values[position] |= bitShift;
    bitShift <<= 1;
    if (bitShift == 256)
    {
      ++position;
      bitShift = 1;
    }
  }
  void fillBits()
  {
    if (bitShift > 1)
      bitShift = 1, ++position;
  }

  size_t getLength() const
    {return values.size();}

  unsigned char getByte(size_t index) const
    {return values[index];}

  void pushByte(unsigned char c)
    {jassert(position < values.size()); values[position++] = c;}

  void pushBytes(unsigned char* data, size_t length)
  {
    jassert(position + length <= values.size());
    memcpy(&values[position], data, length);
    position += length;
  }

  void push32BitInteger(int value)
  {
    memcpy(&values[position], &value, 4);
    position += 4;
  }

  void pushInteger(juce::int64 value)
  {
    memcpy(&values[position], &value, sizeof (juce::int64));
    position += sizeof (juce::int64);
  }

  void pushPointer(const ObjectPtr& object)
  {
    void* pointer = object.get();
    memcpy(&values[position], &pointer, sizeof (void* ));
    position += sizeof (void* );
  }

  size_t computeHashValue() const
  {
    size_t res = 0;
    const unsigned char* ptr = &values[0];
    const unsigned char* lim = ptr + values.size();
    while (ptr < lim)
      res = 31 * res + *ptr++;
    return res;
  }
  
  virtual String toShortString() const
  {
    String res;
    for (size_t i = 0; i < values.size(); ++i)
      res += String::toHexString(values[i]);
    return res;
  }

protected:
  std::vector<unsigned char> values;
  size_t position;
  size_t bitShift;
};

typedef ReferenceCountedObjectPtr<BinaryKey> BinaryKeyPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_BINARY_KEY_H_
