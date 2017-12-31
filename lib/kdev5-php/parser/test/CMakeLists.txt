ecm_add_test(
    lexertest.cpp
    LINK_LIBRARIES
        kdevphpparser
        Qt5::Test
        Qt5::Core
        KDev::Language
)

# hack to make ../phpdebugvisitor.h (generated at compile time)
# use the correct EXPORT tags when compiling in MinGW
if (MINGW)
    add_definitions(-DMAKE_KDEV4PHPPARSER_LIB)
endif (MINGW)
