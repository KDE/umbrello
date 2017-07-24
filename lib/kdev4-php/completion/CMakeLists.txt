add_subdirectory(tests)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(completion_SRCS
    worker.cpp
    item.cpp
    implementationitem.cpp
    includefileitem.cpp
    keyworditem.cpp
    context.cpp
    model.cpp
    helpers.cpp
    codemodelitem.cpp
)



kde4_add_library( kdev4phpcompletion SHARED ${completion_SRCS} )

# hack to make ../parser/phpdebugvisitor.h (generated at compile time)
# use the correct EXPORT tags when compiling in MinGW
if (MINGW)
    add_definitions(-DMAKE_KDEV4PHPPARSER_LIB)
endif (MINGW)

target_link_libraries(kdev4phpcompletion LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    kdev4phpduchain
    kdev4phpparser
 )

install(TARGETS kdev4phpcompletion DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

