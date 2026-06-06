cmake_minimum_required(VERSION 3.16)
project(nomacs-decklink)

# -----------------------------------------------------------------------
# Paths – override any of these in CMakeUser.txt if needed
# -----------------------------------------------------------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/CMakeUser.txt")
    include("${CMAKE_SOURCE_DIR}/CMakeUser.txt")
endif()

# nomacs source headers (DkCore + DkGui + src root all needed transitively)
if(NOT DEFINED NOMACS_INCLUDE_DIRECTORY)
    set(NOMACS_INCLUDE_DIRECTORY
        "${CMAKE_SOURCE_DIR}/../nomacs/ImageLounge/src/DkCore"
        "${CMAKE_SOURCE_DIR}/../nomacs/ImageLounge/src/DkGui"
        "${CMAKE_SOURCE_DIR}/../nomacs/ImageLounge/src")
endif()

# nomacs installed app bundle (we link against the core lib inside it)
if(NOT DEFINED NOMACS_APP)
    set(NOMACS_APP "/Applications/nomacs.app")
endif()

set(NOMACS_FRAMEWORKS_DIR "${NOMACS_APP}/Contents/Frameworks")
set(NOMACS_PLUGINS_DIR    "${NOMACS_APP}/Contents/PlugIns/nomacs")
set(NOMACS_CORE_LIB       "${NOMACS_FRAMEWORKS_DIR}/libnomacsCore-3.22.1.dylib")

# DeckLink SDK root – default: sibling directory in this repo
if(NOT DEFINED DECKLINK_SDK_DIR)
    set(DECKLINK_SDK_DIR "${CMAKE_SOURCE_DIR}/Blackmagic DeckLink SDK 16.0")
endif()
set(DECKLINK_INCLUDE_DIR "${DECKLINK_SDK_DIR}/Mac/include")

# -----------------------------------------------------------------------
# Qt  (pick up the Homebrew Qt6 that nomacs was built against)
# -----------------------------------------------------------------------
if(NOT DEFINED CMAKE_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
endif()

find_package(Qt6 6.0 REQUIRED COMPONENTS Core Gui Widgets)
set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------
set(SOURCES
    src/DkDeckLinkPlugin.cpp
    src/DkDeckLinkViewPort.cpp
    src/DkDeckLinkOutput.cpp
    src/DkDeckLinkDialog.cpp
    # DeckLink SDK dispatch (loads the framework bundle at runtime)
    "${DECKLINK_INCLUDE_DIR}/DeckLinkAPIDispatch.cpp"
)

set(HEADERS
    src/DkDeckLinkPlugin.h
    src/DkDeckLinkViewPort.h
    src/DkDeckLinkOutput.h
    src/DkDeckLinkDialog.h
)

qt_add_resources(PLUGIN_RCC src/nomacsPlugin.qrc)

# -----------------------------------------------------------------------
# Plugin target
# -----------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${PLUGIN_RCC})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${DECKLINK_INCLUDE_DIR}"
    ${NOMACS_INCLUDE_DIRECTORY}
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_PLUGIN
    QT_SHARED
    QT_DLL
)

# Link Qt + CoreFoundation (for DeckLink API dispatch)
target_link_libraries(${PROJECT_NAME}
    ${QT_LIBS}
    "-framework CoreFoundation"
    # Link nomacs core so DkImageContainer, DkPluginInterface etc. resolve
    "${NOMACS_CORE_LIB}"
)

# At runtime the plugin is loaded from inside the app bundle, where @rpath
# resolves to the bundle's Frameworks directory.
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "@loader_path/../../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
    MACOSX_RPATH TRUE
)

# -----------------------------------------------------------------------
# Fix Qt references: Homebrew links with absolute paths; we need @rpath
# so the plugin uses the same Qt instance that nomacs bundles.
# -----------------------------------------------------------------------
set(QT_BREW_PREFIX /opt/homebrew/opt/qtbase/lib)

foreach(_fw Widgets Gui Core)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND install_name_tool -change
            "${QT_BREW_PREFIX}/Qt${_fw}.framework/Versions/A/Qt${_fw}"
            "@rpath/Qt${_fw}.framework/Versions/A/Qt${_fw}"
            "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Fixing Qt${_fw} rpath in plugin"
    )
endforeach()

# -----------------------------------------------------------------------
# Install directly into the running nomacs app bundle
# -----------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION "${NOMACS_PLUGINS_DIR}"
)
