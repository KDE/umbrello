include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

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
    duchaindebug.cpp
)

add_library( kdevphpduchain SHARED ${duchain_SRCS} )

generate_export_header(kdevphpduchain EXPORT_MACRO_NAME KDEVPHPDUCHAIN_EXPORT
                                       EXPORT_FILE_NAME phpduchainexport.h)

target_link_libraries(kdevphpduchain LINK_PRIVATE
    KDev::Interfaces
    KDev::Language
    KDev::Shell
    KDev::Project
    kdevphpparser
)

install(TARGETS kdevphpduchain DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

