
#include <clang-c/Index.h>

#include <iostream>

#include "basic_types.h"

using namespace std;

i32 main(i32 argc, char** argv) {

    if(argc < 2) {
        cout << "Incorrect usage." << endl;
        return -1;
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(index, argv[1], nullptr, 0, nullptr, 0, CXTranslationUnit_None);

    if (unit == nullptr) {
        cout << "Unable to parse translation unit." << endl;
        return -1;
    }

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    cout << "Done!" << endl;

    return 0;
}
