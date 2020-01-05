if (BUILD_KF5)
    add_library(KDevTests STATIC tests/testcore.cpp tests/autotestshell.cpp)
    target_include_directories(KDevTests PUBLIC tests)
    target_link_libraries(KDevTests
        PUBLIC
            KDev::Shell
            #KDev::Language
            #KDev::Project
            #KDev::Debugger
        PRIVATE
    )

    add_library(KDev::Tests ALIAS KDevTests)
endif()
