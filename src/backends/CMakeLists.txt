function(add_backend name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        JSON "${name}.json"
        INSTALL_NAMESPACE "cantor/backends")

    target_link_libraries("cantor_${name}"
        cantorlibs)

endfunction()

add_subdirectory(maxima)
add_subdirectory(octave)
add_subdirectory(scilab)

set(cantor_HELP
   qthelpconfig.cpp
)

ki18n_wrap_ui(cantor_HELP qthelpconfig.ui)
ki18n_wrap_ui(cantor_HELP qthelpconfigeditdialog.ui)

add_library(cantor_help STATIC ${cantor_HELP} backendsettingswidget.cpp)
target_link_libraries(cantor_help KF5::SyntaxHighlighting KF5::I18n KF5::IconThemes KF5::NewStuff KF5::KIOWidgets Qt5::Help)

if(NOT WIN32)
    add_subdirectory(sage)
endif(NOT WIN32)

set_package_properties(Analitza5 PROPERTIES DESCRIPTION "A library provided by KAlgebra."
    URL "https://edu.kde.org/kalgebra/"
    TYPE OPTIONAL
    PURPOSE "Backend to use KAlgebra with Cantor.")

find_package(Analitza5)
if(Analitza5_FOUND)
    add_subdirectory(kalgebra)
endif(Analitza5_FOUND)

if(NOT WIN32)
    set_package_properties(R PROPERTIES DESCRIPTION "A free software environment for statistical computing and graphics."
        URL "https://www.r-project.org/"
        TYPE OPTIONAL
        PURPOSE "Backend to use R with Cantor.")

    find_package(R)
    if(R_FOUND)
        add_subdirectory(R)
    endif(R_FOUND)
endif(NOT WIN32)

set_package_properties(Qalculate PROPERTIES DESCRIPTION "A multi-purpose desktop calculator with support for customizable functions, units and arbitrary precision."
    URL "https://qalculate.github.io/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Qalculate with Cantor.")

find_package(Qalculate)
if(QALCULATE_FOUND)
    add_subdirectory(qalculate)
endif(QALCULATE_FOUND)

set_package_properties(PythonLibs3 PROPERTIES DESCRIPTION "A powerful dynamic programming language."
    URL "https://www.python.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Python with Cantor.")

find_package(PythonLibs3)
if(PYTHONLIBS3_FOUND)
    add_subdirectory(python)
endif(PYTHONLIBS3_FOUND)

set_package_properties(LuaJIT PROPERTIES DESCRIPTION "A lightweight, extensible programming language (luajit implementation)."
    URL "https://www.lua.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Lua (luajit2) with Cantor.")

find_package(LuaJIT)
if(LUAJIT_FOUND)
    add_subdirectory(lua)
endif(LUAJIT_FOUND)

set_package_properties(
    Julia
    PROPERTIES DESCRIPTION "A high-level, high-performance dynamic programming language for technical computing"
    URL "https://julialang.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Julia with Cantor.")

find_package(Julia)
if(Julia_FOUND)
    add_subdirectory(julia)
endif()
