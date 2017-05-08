add_subdirectory(tests)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(duchain_SRCS
    types/integraltypeextended.cpp
    types/structuretype.cpp
    builders/predeclarationbuilder.cpp
    builders/declarationbuilder.cpp
    builders/contextbuilder.cpp
    builders/usebuilder.cpp
    builders/typebuilder.cpp
    editorintegrator.cpp
    dumptypes.cpp
    expressionvisitor.cpp
    expressionparser.cpp
    expressionevaluationresult.cpp
    helper.cpp
    phpducontext.cpp
    declarations/variabledeclaration.cpp
    declarations/classmethoddeclaration.cpp
    declarations/classdeclaration.cpp
    declarations/functiondeclaration.cpp
    declarations/namespacedeclaration.cpp
    declarations/namespacealiasdeclaration.cpp
    declarations/traitmethodaliasdeclaration.cpp
    declarations/traitmemberaliasdeclaration.cpp
    navigation/navigationwidget.cpp
    navigation/declarationnavigationcontext.cpp
    navigation/includenavigationcontext.cpp
    navigation/magicconstantnavigationcontext.cpp
    completioncodemodel.cpp
    )


kde4_add_library( kdev4phpduchain SHARED ${duchain_SRCS} )

# hack to make ../parser/phpdebugvisitor.h (generated at compile time)
# use the correct EXPORT tags when compiling in MinGW
if (MINGW)
    add_definitions(-DMAKE_KDEV4PHPPARSER_LIB)
endif (MINGW)

target_link_libraries(kdev4phpduchain LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_SHELL_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdev4phpparser
 )

install(TARGETS kdev4phpduchain DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

