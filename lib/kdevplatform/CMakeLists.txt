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
            # fix kdevelop-5.2.1 issue, see https://bugs.kde.org/show_bug.cgi?id=424794
            KF5::ThreadWeaver
    )

    add_library(KDev::Tests ALIAS KDevTests)
endif()
