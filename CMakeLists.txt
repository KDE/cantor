cmake_minimum_required (VERSION 3.13 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "0")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(cantor VERSION ${RELEASE_SERVICE_VERSION})

set(KF5_MIN_VERSION "5.70.0")

find_package(ECM 5.15.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${cantor_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

set( SharedMimeInfo_MINIMUM_VERSION "1.3" ) #TODO: What is the minimum required version?
find_package(SharedMimeInfo ${SharedMimeInfo_MINIMUM_VERSION} REQUIRED)

find_package(Qt5 5.8.0 CONFIG REQUIRED
    Core
    Widgets
    PrintSupport
    Svg
    Xml
    XmlPatterns
    Test
    Help
    WebEngine
    WebEngineWidgets)

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
    SyntaxHighlighting
    TextWidgets
    KIO
    XmlGui
    I18n
)

find_package(Poppler "0.62.0" REQUIRED COMPONENTS Qt5)


if(NOT WIN32)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED Pty)
endif()

option(USE_LIBSPECTRE "Use libspectre library" ON)

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMAddAppIcon)
include(GenerateExportHeader)

add_subdirectory(thirdparty)
# Need to set this here, because variable can't move from subdirectory (thirdparty) to upper cmake scope
set(Discount_FOUND TRUE)

if(NOT WIN32 AND USE_LIBSPECTRE)
    set_package_properties(LibSpectre PROPERTIES DESCRIPTION "A PostScript rendering library"
        URL "https://www.freedesktop.org/wiki/Software/libspectre/"
        TYPE OPTIONAL
        PURPOSE "Support for rendering EPS files in Cantor")

    find_package(LibSpectre)
    if(LIBSPECTRE_FOUND)
        set(WITH_EPS On)
    else(LIBSPECTRE_FOUND)
        set(WITH_EPS Off)
    endif(LIBSPECTRE_FOUND)

else()
    set(WITH_EPS Off)
endif()

#[[
find_package(Discount 2.2.0)
set_package_properties(Discount PROPERTIES DESCRIPTION "A C implementation of the Markdown markup language"
    URL "https://www.pell.portland.or.us/~orc/Code/discount/"
    TYPE OPTIONAL
    PURPOSE "Used for Markdown entries in Cantor")
]]#

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")

kde_enable_exceptions()

# FIXME Looks like docbook on Windows don't buildable at all, so it are temporarily disabled
if(NOT WIN32)
    add_subdirectory(doc)
endif(NOT WIN32)
add_subdirectory(src)
add_subdirectory(icons)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/Cantor")
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CantorConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CantorConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}/
    #PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR
)

ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX CANTOR
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/cantor_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/CantorConfigVersion.cmake"
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/CantorConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/CantorConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT CantorTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE CantorTargets.cmake
    NAMESPACE Cantor::
)


ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()
install(FILES org.kde.cantor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
