add_subdirectory(test)

set(parser_STAT_SRCS
    phplexer.cpp
    parsesession.cpp
    )

kdevpgqt_generate(parser_SRCS php NAMESPACE Php DEBUG_VISITOR TOKEN_TEXT
    "${php_SOURCE_DIR}/parser/php.g"
    "${php_SOURCE_DIR}/parser/phplexer.h"
)

if(NO_INSTALL)
    kde4_add_library( kdev4phpparser STATIC ${parser_SRCS} ${parser_STAT_SRCS} )
else()
    kde4_add_library( kdev4phpparser SHARED ${parser_SRCS} ${parser_STAT_SRCS} )
endif()
target_link_libraries( kdev4phpparser LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDE4_KTEXTEDITOR_LIBS}
)

kde4_add_executable( php-parser main.cpp )

# hack to make phpdebugvisitor.h (generated at compile time)
# use the correct EXPORT tags when compiling in MinGW
if (MINGW)
    add_definitions(-DMAKE_KDEV4PHPPARSER_LIB)
endif (MINGW)

target_link_libraries(php-parser
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    kdev4phpparser
)

if(NOT NO_INSTALL)
    install(TARGETS kdev4phpparser DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()
