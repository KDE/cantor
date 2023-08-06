cmake_minimum_required (VERSION 3.13 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(cantor VERSION ${RELEASE_SERVICE_VERSION})

if (QT_MAJOR_VERSION STREQUAL "6")
	set(QT_MIN_VERSION "6.4.0")
	set(KF_MIN_VERSION "5.240.0")
	set(KF_MAJOR_VERSION "6")
else()
	set(QT_MIN_VERSION "5.8.0")
	set(KF_MIN_VERSION "5.91.0")
	set(KF_MAJOR_VERSION "5")
endif()

find_package(ECM 5.15.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${cantor_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddAppIcon)
include(KDEClangFormat)
include(GenerateExportHeader)
include(FeatureSummary)

set( SharedMimeInfo_MINIMUM_VERSION "1.3" ) #TODO: What is the minimum required version?
find_package(SharedMimeInfo ${SharedMimeInfo_MINIMUM_VERSION} REQUIRED)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED
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

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED
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

find_package(Poppler "0.62.0" REQUIRED COMPONENTS Qt${QT_MAJOR_VERSION})

option(USE_LIBSPECTRE "Use libspectre library" ON)

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

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)
if (NOT WIN32)
    # Strict iterators can't be used on Windows, they lead to a link error
    # when application code iterates over a QVector<QPoint> for instance, unless
    # Qt itself was also built with strict iterators.
    # See example at https://bugreports.qt.io/browse/AUTOSUITE-946
    add_definitions(-DQT_STRICT_ITERATORS)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

   # both gcc & clang support it since the same version
   if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.0)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wzero-as-null-pointer-constant" )
   endif()
endif()

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

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
