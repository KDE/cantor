function(add_backend name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        INSTALL_NAMESPACE "cantor_plugins/backends")

    target_link_libraries("cantor_${name}"
        cantorlibs)

endfunction()

add_subdirectory(maxima)
add_subdirectory(octave)
add_subdirectory(scilab)

if (ENABLE_EMBEDDED_DOCUMENTATION)
    set(cantor_HELP
        qthelpconfig.cpp
    )

    ki18n_wrap_ui(cantor_HELP qthelpconfig.ui)
    ki18n_wrap_ui(cantor_HELP qthelpconfigeditdialog.ui)

    add_library(cantor_help STATIC ${cantor_HELP})
    set_target_properties(cantor_help PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(cantor_help KF6::SyntaxHighlighting KF6::I18n KF6::IconWidgets KF6::NewStuffWidgets KF6::KIOWidgets Qt6::Help)
endif()

if(NOT WIN32)
    add_subdirectory(sage)
endif(NOT WIN32)

set_package_properties(Analitza5 PROPERTIES DESCRIPTION "A library provided by KAlgebra."
    URL "https://edu.kde.org/kalgebra/"
    TYPE OPTIONAL
    PURPOSE "Backend to use KAlgebra with Cantor.")

find_package(Analitza6)
if(Analitza6_FOUND)
    add_subdirectory(kalgebra)
endif(Analitza6_FOUND)

set_package_properties(R PROPERTIES DESCRIPTION "A free software environment for statistical computing and graphics."
  URL "https://www.r-project.org/"
  TYPE OPTIONAL
  PURPOSE "Backend to use R with Cantor.")

find_package(R)
if(R_FOUND)
  add_subdirectory(R)
endif(R_FOUND)

set_package_properties(Qalculate PROPERTIES DESCRIPTION "A multi-purpose desktop calculator with support for customizable functions, units and arbitrary precision."
    URL "https://qalculate.github.io/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Qalculate with Cantor.")

# set the minimal version to 3.11 (release date 22.06.2020) since this is the version where the colored output
# was introduced and we don't want to deal with two different formattings and switch off the color completely
# when starting qcalc
set(QALCULATE_MIN_VERSION 3.11.0)
find_package(Qalculate)
if(QALCULATE_FOUND)
    add_subdirectory(qalculate)
endif(QALCULATE_FOUND)

set_package_properties(Python3 PROPERTIES TYPE OPTIONAL
    PURPOSE "Backend to use Python with Cantor.")

find_package(Python3 COMPONENTS Development)
if(Python3_FOUND)
    add_subdirectory(python)
endif()

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
