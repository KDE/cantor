project(cantor)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

add_definitions(-DMAKE_CANTORLIBS_LIB)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "17")
set (KDE_APPLICATIONS_VERSION_MINOR "11")
set (KDE_APPLICATIONS_VERSION_MICRO "80")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

set(KF5_MIN_VERSION "5.15.0")

find_package(ECM 5.15.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${cantor_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 CONFIG REQUIRED
    Core
    Widgets
    PrintSupport
    Svg
    Xml
    XmlPatterns
    Test)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    Config
    Crash
    Completion
    DocTools
    NewStuff
    IconThemes
    TextEditor
    CoreAddons
    Archive
    Parts
    TextWidgets
    KIO
    XmlGui
    I18n)

if(NOT WIN32)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED Pty)
endif()

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMAddAppIcon)

if(NOT WIN32)
    set_package_properties(LibSpectre PROPERTIES DESCRIPTION "A PostScript rendering library"
        URL "http://libspectre.freedesktop.org/wiki/"
        TYPE OPTIONAL
        PURPOSE "Support for rendering EPS files in Cantor")

    find_package(LibSpectre)
    if(LibSpectre_FOUND)
        set(WITH_EPS On)
    else(LibSpectre_FOUND)
        set(WITH_EPS Off)
    endif(LibSpectre_FOUND)

else(NOT WIN32)
    set(WITH_EPS Off)
endif(NOT WIN32)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
kde_enable_exceptions()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(icons)

install(FILES org.kde.cantor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
