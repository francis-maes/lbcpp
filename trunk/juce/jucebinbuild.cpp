/*
  ==============================================================================

   Utility to turn a bunch of binary files into a .cpp file and .h file full of
   data so they can be built directly into an executable.

   Copyright 2004 by Julian Storer.

   Use this code at your own risk! It carries no warranty!

  ==============================================================================
*/
#define JUCE_DLL
#include "juce_amalgamated.h"
#include <map>
using namespace juce;

//==============================================================================
static std::map<String, String> dataFiles;

static int addFile (const File& file,
                    const String& classname, 
                    OutputStream& headerStream,
                    OutputStream& cppStream)
{
    MemoryBlock mb;
    file.loadFileAsData (mb);

    const String name (file.getFileName().toLowerCase()
                           .replaceCharacter (' ', '_')
                           .replaceCharacter ('.', '_')
                           .retainCharacters (JUCE_T("abcdefghijklmnopqrstuvwxyz_0123456789")));

    dataFiles[file.getFileName()] = name;
    printf ("Adding %s: %d bytes\n", 
            (const char*) name,
            mb.getSize());

    headerStream.printf ("    extern const char*  %s;\r\n"
                         "    const int           %sSize = %d;\r\n\r\n",
                         (const char*) name,
                         (const char*) name,
                         mb.getSize());

    static int tempNum = 0;

    cppStream.printf ("static const unsigned char temp%d[] = {", ++tempNum);

    int i = 0;
    while (i < mb.getSize() - 1)
    {
        if ((i % 40) != 39)
            cppStream.printf ("%d,", mb[i] & 0xff);
        else
            cppStream.printf ("%d,\r\n  ", mb[i] & 0xff);

        ++i;
    }

    cppStream.printf ("%d,0,0};\r\n", mb[i] & 0xff);

    cppStream.printf ("const char* %s::%s = (const char*)temp%d;\r\n\r\n",
                      (const char*) classname, 
                      (const char*) name, 
                      tempNum);

    return mb.getSize();
}

//==============================================================================
int main (int argc, char* argv[])
{
    printf ("\n BinaryBuilder! Copyright 2004 by Julian Storer - www.rawmaterialsoftware.com\n\n");

    if (argc < 4)
    {
        printf (" Usage: BinaryBuilder  source_directory target_directory target_class_name [additional include] [additional include] ...\n\n");
        printf (" BinaryBuilder will find all files in the source directory, and encode them\n");
        printf (" into two files called (targetclassname).cpp and (targetclassname).h, which it\n");
        printf (" will write into the target directory supplied.\n\n");
        printf (" Any files in sub-directories of the source directory will be put into the\n");
        printf (" resultant class, but #ifdef'ed out using the name of the sub-directory (hard to\n");
        printf (" explain, but obvious when you try it...)\n");

        return 0;
    }

    // because we're not using the proper application startup procedure, we need to call
    // this explicitly here to initialise some of the time-related stuff..
    SystemStats::initialiseStats();

    const File sourceDirectory (File::getCurrentWorkingDirectory().getChildFile (argv[1]));

    if (! sourceDirectory.isDirectory())
    {
        String error ("Source directory doesn't exist: ");
        error << sourceDirectory.getFullPathName() << "\n\n";

        printf ((const char*) error);
        return 0;
    }

    const File destDirectory (File::getCurrentWorkingDirectory().getChildFile (argv[2]));

    if (! destDirectory.isDirectory())
    {
        String error ("Destination directory doesn't exist: ");
        error << destDirectory.getFullPathName() << "\n\n";

        printf ((const char*) error);
        return 0;
    }

    String className (argv[3]);
    className = className.trim();

    const File headerFile (destDirectory.getChildFile (className).withFileExtension (T(".h")));
    const File cppFile    (destDirectory.getChildFile (className).withFileExtension (T(".cpp")));

    String message;
    message << "Creating " << headerFile.getFullPathName() 
            << " and " << cppFile.getFullPathName()
            << " from files in " << sourceDirectory.getFullPathName() 
            << "...\n\n";

    printf ((const char*) message);

    OwnedArray<File> files;
    sourceDirectory.findChildFiles (files, File::findFiles, false, "*");

    if (files.size() == 0)
    {
        String error ("Didn't find any source files in: ");
        error << sourceDirectory.getFullPathName() << "\n\n";
        printf ((const char*) error);
        return 0;
    }

    headerFile.deleteFile();
    cppFile.deleteFile();

    OutputStream* header = headerFile.createOutputStream();

    if (header == 0)
    {
        String error ("Couldn't open ");
        error << headerFile.getFullPathName() << " for writing\n\n";
        printf ((const char*) error);
        return 0;
    }

    OutputStream* cpp = cppFile.createOutputStream();

    if (cpp == 0)
    {
        String error ("Couldn't open ");
        error << cppFile.getFullPathName() << " for writing\n\n";
        printf ((const char*) error);
        return 0;
    }

    header->printf ("/* (Auto-generated binary data file). */\r\n\r\n"
                    "#pragma once\r\n"
                    "#ifndef BINARY_%s_H\r\n"
                    "#define BINARY_%s_H\r\n\r\n"
                    "namespace %s\r\n"
                    "{\r\n",
                    (const char*) className.toUpperCase(),
                    (const char*) className.toUpperCase(),
                    (const char*) className);

	if (argc > 4)
	{
		for (int i = 4; i < argc; ++i)
			cpp->printf("#include \"%s\"\r\n\r\n", argv[i]);
	}

    cpp->printf ("/* (Auto-generated binary data file). */\r\n\r\n"
                 "#include \"%s.h\"\r\n\r\n",
                 (const char*) className);

    int totalBytes = 0;

    for (int i = 0; i < files.size(); ++i)
    {
        const File file (*(files[i]));

        if (!file.getFileName().endsWithIgnoreCase (JUCE_T(".scc")) &&
            file != headerFile && file != cppFile)
        {
            if (file.getParentDirectory() != sourceDirectory)
            {
                header->printf ("  #ifdef %s\r\n", (const char*) file.getParentDirectory().getFileName().toUpperCase());
                cpp->printf      ("#ifdef %s\r\n", (const char*) file.getParentDirectory().getFileName().toUpperCase());

                totalBytes += addFile (file, className, *header, *cpp);

                header->printf ("  #endif\r\n");
                cpp->printf ("#endif\r\n");
            }
            else
            {
                totalBytes += addFile (file, className, *header, *cpp);
            }
            
        }
    }


    header->printf("    extern const char* get(const String& fileName, int& size);\r\n");
    *cpp << T("const char* ") + className + T("::get(const String& fileName, int& size)\r\n{\r\n");
    for (std::map<String, String>::const_iterator it = dataFiles.begin(); it != dataFiles.end(); ++it)
      *cpp << T("  if (fileName == T(") << it->first.quoted() << T(")) {size = ") << it->second << T("Size; return ") << it->second << ";}\r\n";
    *cpp << T("  size = 0; return NULL;\r\n");
    cpp->printf("}\r\n");

    header->printf ("};\r\n\r\n"
                    "#endif\r\n");

    delete header;
    delete cpp;

    printf ("\n Total size of binary data: %d bytes\n", totalBytes);

    return 0;
}
