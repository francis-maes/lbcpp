// francis
#ifdef LBCPP_USER_INTERFACE
#define JUCE_DLL_BUILD
#include "juce_amalgamated.h"

#include "juce_LuaCodeTokeniser.h"
BEGIN_JUCE_NAMESPACE



LuaCodeTokeniser::LuaCodeTokeniser()
{
}

LuaCodeTokeniser::~LuaCodeTokeniser()
{
}

//==============================================================================
namespace LuaTokeniser
{

static bool isIdentifierStart (const tchar c) throw()
{
    return CharacterFunctions::isLetter (c)
            || c == JUCE_T('_');
}

static bool isIdentifierBody (const tchar c) throw()
{
    return CharacterFunctions::isLetter (c)
            || CharacterFunctions::isDigit (c)
            || c == JUCE_T('_');
}

static int parseIdentifier (CodeDocument::Iterator& source) throw()
{
    static const tchar* keywords2Char[] =
       { JUCE_T("do"), JUCE_T("if"), JUCE_T("in"), JUCE_T("or"), 0 };

    static const tchar* keywords3Char[] =
      { JUCE_T("and"), JUCE_T("end"), JUCE_T("for"), JUCE_T("nil"), JUCE_T("not"), 0};

    static const tchar* keywords4Char[] =
      { JUCE_T("else"), JUCE_T("then"), JUCE_T("true"), 0};

    static const tchar* keywords5Char[] =
      { JUCE_T("break"), JUCE_T("false"), JUCE_T("local"), JUCE_T("until"), JUCE_T("while"), 0};

    static const tchar* keywords6Char[] =
      { JUCE_T("elseif"), JUCE_T("repeat"), JUCE_T("return"), 0};
        
    static const tchar* keywordsOther[] =
        { JUCE_T("function"), JUCE_T("subspecified"), JUCE_T("parameter"), JUCE_T("derivable"), 0 };

    int tokenLength = 0;
    tchar possibleIdentifier [19];

    while (isIdentifierBody (source.peekNextChar()))
    {
        const tchar c = source.nextChar();

        if (tokenLength < numElementsInArray (possibleIdentifier) - 1)
            possibleIdentifier [tokenLength] = c;

        ++tokenLength;
    }

    if (tokenLength > 1 && tokenLength <= 16)
    {
        possibleIdentifier [tokenLength] = 0;
        const tchar** k;

        switch (tokenLength)
        {
            case 2:     k = keywords2Char; break;
            case 3:     k = keywords3Char; break;
            case 4:     k = keywords4Char; break;
            case 5:     k = keywords5Char; break;
            case 6:     k = keywords6Char; break;
            default:    k = keywordsOther; break;
        }

        int i = 0;
        while (k[i] != 0)
        {
            if (k[i][0] == possibleIdentifier[0] && CharacterFunctions::compare (k[i], possibleIdentifier) == 0)
                return LuaCodeTokeniser::tokenType_builtInKeyword;

            ++i;
        }
    }

    return LuaCodeTokeniser::tokenType_identifier;
}

static bool skipNumberSuffix (CodeDocument::Iterator& source)
{
    const juce_wchar c = source.peekNextChar();
    if (c == 'l' || c == 'L' || c == 'u' || c == 'U')
        source.skip();

    if (CharacterFunctions::isLetterOrDigit (source.peekNextChar()))
        return false;

    return true;
}

static bool isHexDigit (const juce_wchar c) throw()
{
    return (c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
}

static bool parseHexLiteral (CodeDocument::Iterator& source) throw()
{
    if (source.nextChar() != '0')
        return false;

    juce_wchar c = source.nextChar();
    if (c != 'x' && c != 'X')
        return false;

    int numDigits = 0;
    while (isHexDigit (source.peekNextChar()))
    {
        ++numDigits;
        source.skip();
    }

    if (numDigits == 0)
        return false;

    return skipNumberSuffix (source);
}

static bool isOctalDigit (const juce_wchar c) throw()
{
    return c >= '0' && c <= '7';
}

static bool parseOctalLiteral (CodeDocument::Iterator& source) throw()
{
    if (source.nextChar() != '0')
        return false;

    if (! isOctalDigit (source.nextChar()))
         return false;

    while (isOctalDigit (source.peekNextChar()))
        source.skip();

    return skipNumberSuffix (source);
}

static bool isDecimalDigit (const juce_wchar c) throw()
{
    return c >= '0' && c <= '9';
}

static bool parseDecimalLiteral (CodeDocument::Iterator& source) throw()
{
    int numChars = 0;
    while (isDecimalDigit (source.peekNextChar()))
    {
        ++numChars;
        source.skip();
    }

    if (numChars == 0)
        return false;

    return skipNumberSuffix (source);
}

static bool parseFloatLiteral (CodeDocument::Iterator& source) throw()
{
    int numDigits = 0;

    while (isDecimalDigit (source.peekNextChar()))
    {
        source.skip();
        ++numDigits;
    }

    const bool hasPoint = (source.peekNextChar() == '.');

    if (hasPoint)
    {
        source.skip();

        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numDigits;
        }
    }

    if (numDigits == 0)
        return false;

    juce_wchar c = source.peekNextChar();
    const bool hasExponent = (c == 'e' || c == 'E');

    if (hasExponent)
    {
        source.skip();

        c = source.peekNextChar();
        if (c == '+' || c == '-')
            source.skip();

        int numExpDigits = 0;
        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numExpDigits;
        }

        if (numExpDigits == 0)
            return false;
    }

    c = source.peekNextChar();
    if (c == 'f' || c == 'F')
        source.skip();
    else if (! (hasExponent || hasPoint))
        return false;

    return true;
}

static int parseNumber (CodeDocument::Iterator& source)
{
    const CodeDocument::Iterator original (source);

    if (parseFloatLiteral (source))
        return LuaCodeTokeniser::tokenType_floatLiteral;

    source = original;

    if (parseHexLiteral (source))
        return LuaCodeTokeniser::tokenType_integerLiteral;

    source = original;

    if (parseOctalLiteral (source))
        return LuaCodeTokeniser::tokenType_integerLiteral;

    source = original;

    if (parseDecimalLiteral (source))
        return LuaCodeTokeniser::tokenType_integerLiteral;

    source = original;
    source.skip();

    return LuaCodeTokeniser::tokenType_error;
}

static void skipQuotedString (CodeDocument::Iterator& source) throw()
{
    const juce_wchar quote = source.nextChar();

    for (;;)
    {
        const juce_wchar c = source.nextChar();

        if (c == quote || c == 0)
            break;

        if (c == '\\')
            source.skip();
    }
}

static void skipComment (CodeDocument::Iterator& source) throw()
{
    bool lastWasStar = false;

    for (;;)
    {
        const juce_wchar c = source.nextChar();

        if (c == 0 || (c == JUCE_T('/') && lastWasStar))
            break;

        lastWasStar = (c == '*');
    }
}

}

//==============================================================================
int LuaCodeTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    int result = tokenType_error;
    source.skipWhitespace();

    tchar firstChar = source.peekNextChar();

    switch (firstChar)
    {
    case 0:
        source.skip();
        break;

    case JUCE_T('0'):
    case JUCE_T('1'):
    case JUCE_T('2'):
    case JUCE_T('3'):
    case JUCE_T('4'):
    case JUCE_T('5'):
    case JUCE_T('6'):
    case JUCE_T('7'):
    case JUCE_T('8'):
    case JUCE_T('9'):
        result = LuaTokeniser::parseNumber (source);
        break;

    case JUCE_T('.'):
        result = LuaTokeniser::parseNumber (source);

        if (result == tokenType_error)
            result = tokenType_punctuation;

        break;

    case JUCE_T(','):
    case JUCE_T(';'):
    case JUCE_T(':'):
        source.skip();
        result = tokenType_punctuation;
        break;

    case JUCE_T('('):
    case JUCE_T(')'):
    case JUCE_T('{'):
    case JUCE_T('}'):
    case JUCE_T('['):
    case JUCE_T(']'):
        source.skip();
        result = tokenType_bracket;
        break;

    case JUCE_T('"'):
    case JUCE_T('\''):
        LuaTokeniser::skipQuotedString (source);
        result = tokenType_stringLiteral;
        break;

    case JUCE_T('+'):
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == JUCE_T('+'))
            source.skip();
        else if (source.peekNextChar() == JUCE_T('='))
            source.skip();

        break;

    case JUCE_T('-'):

        source.skip();
        if (source.peekNextChar() == JUCE_T('-'))
        {
            result = tokenType_comment;
            source.skipToEndOfLine();
        }
        else
        {
          result = LuaTokeniser::parseNumber (source);

          if (result == tokenType_error)
          {
              result = tokenType_operator;

              if (source.peekNextChar() == JUCE_T('-'))
                  source.skip();
              else if (source.peekNextChar() == JUCE_T('='))
                  source.skip();
          }
        }
        break;

    case JUCE_T('*'):
    case JUCE_T('%'):
    case JUCE_T('='):
    case JUCE_T('!'):
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == JUCE_T('='))
            source.skip();

        break;

    case JUCE_T('/'):
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('/'))
        {
            result = tokenType_comment;
            source.skipToEndOfLine();
        }
        else if (source.peekNextChar() == JUCE_T('*'))
        {
            source.skip();
            result = tokenType_comment;
            LuaTokeniser::skipComment (source);
        }

        break;

    case JUCE_T('?'):
    case JUCE_T('~'):
        source.skip();
        result = tokenType_operator;
        break;

    case JUCE_T('<'):
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('<'))
        {
            source.skip();

            if (source.peekNextChar() == JUCE_T('='))
                source.skip();
        }

        break;

    case JUCE_T('>'):
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('<'))
        {
            source.skip();

            if (source.peekNextChar() == JUCE_T('='))
                source.skip();
        }

        break;

    case JUCE_T('|'):
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('|'))
        {
            source.skip();

            if (source.peekNextChar() == JUCE_T('='))
                source.skip();
        }

        break;

    case JUCE_T('&'):
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('&'))
        {
            source.skip();

            if (source.peekNextChar() == JUCE_T('='))
                source.skip();
        }

        break;

    case JUCE_T('^'):
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == JUCE_T('='))
        {
            source.skip();
        }
        else if (source.peekNextChar() == JUCE_T('^'))
        {
            source.skip();

            if (source.peekNextChar() == JUCE_T('='))
                source.skip();
        }

        break;

    /*case JUCE_T('#'):
        result = tokenType_preprocessor;
        source.skipToEndOfLine();
        break;*/

    default:
        if (LuaTokeniser::isIdentifierStart (firstChar))
            result = LuaTokeniser::parseIdentifier (source);
        else
            source.skip();

        break;
    }

    //jassert (result != tokenType_unknown);
    return result;
}

const StringArray LuaCodeTokeniser::getTokenTypes()
{
    StringArray s;
    s.add ("Error");
    s.add ("Comment");
    s.add ("C++ keyword");
    s.add ("Identifier");
    s.add ("Integer literal");
    s.add ("Float literal");
    s.add ("String literal");
    s.add ("Operator");
    s.add ("Bracket");
    s.add ("Punctuation");
    s.add ("Preprocessor line");
    return s;
}

const Colour LuaCodeTokeniser::getDefaultColour (const int tokenType)
{
    const uint32 colours[] =
    {
        0xffcc0000,  // error
        0xff00aa00,  // comment
        0xff0000cc,  // keyword
        0xff000000,  // identifier
        0xff880000,  // int literal
        0xff885500,  // float literal
        0xff990099,  // string literal
        0xff225500,  // operator
        0xff000055,  // bracket
        0xff004400,  // punctuation
        0xff660000   // preprocessor
    };

    if (tokenType >= 0 && tokenType < numElementsInArray (colours))
        return Colour (colours [tokenType]);

    return Colours::black;
}


END_JUCE_NAMESPACE
#endif // !LBCPP_USER_INTERFACE
