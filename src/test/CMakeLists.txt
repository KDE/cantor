set(worksheettest_SRCS
    ../worksheet.cpp
    ../worksheetview.cpp
    ../worksheetentry.cpp
    ../worksheettextitem.cpp
    ../worksheetimageitem.cpp
    ../commandentry.cpp
    ../textentry.cpp
    ../markdownentry.cpp
    ../pagebreakentry.cpp
    ../imageentry.cpp
    ../latexentry.cpp
    ../placeholderentry.cpp
    ../horizontalruleentry.cpp
    ../hierarchyentry.cpp
    ../worksheetcursor.cpp
    ../searchbar.cpp
    ../actionbar.cpp
    ../worksheettoolbutton.cpp
    ../imagesettingsdialog.cpp
    ../scripteditor/scripteditorwidget.cpp
    ../resultitem.cpp
    ../textresultitem.cpp
    ../imageresultitem.cpp
    ../animationresultitem.cpp
    ../loadedexpression.cpp
    ../animation.cpp
    ../mathrender.cpp
    ../mathrendertask.cpp
    ../worksheetcontrolitem.cpp
    worksheet_test.cpp)

ki18n_wrap_ui(worksheettest_SRCS ../imagesettings.ui)
ki18n_wrap_ui(worksheettest_SRCS ../standardsearchbar.ui)
ki18n_wrap_ui(worksheettest_SRCS ../extendedsearchbar.ui)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
configure_file("data/Lecture-2B-Single-Atom-Lasing.ipynb" data COPYONLY)
configure_file("data/AEC.04 - Evolutionary Strategies and Covariance Matrix Adaptation.ipynb" data COPYONLY)
configure_file("data/Population_Genetics.ipynb" data COPYONLY)
configure_file("data/A Reaction-Diffusion Equation Solver in Python with Numpy.ipynb" data COPYONLY)
configure_file("data/Automata and Computability using Jupyter.ipynb" data COPYONLY)
configure_file("data/Cue Combination with Neural Populations .ipynb" data COPYONLY)
configure_file("data/Transformation2D.ipynb" data COPYONLY)
configure_file("data/TestMarkdownAttachment.ipynb" data COPYONLY)
configure_file("data/TestEntryLoad1.ipynb" data COPYONLY)
configure_file("data/TestEntryLoad2.ipynb" data COPYONLY)
configure_file("data/TestResultsLoad.ipynb" data COPYONLY)
configure_file("data/TestNotebookWithJson.ipynb" data COPYONLY)
configure_file("data/TestNotebookWithModJson.ipynb" data COPYONLY)
configure_file("data/EmptyPythonWorksheet.cws" data COPYONLY)
configure_file("data/TestCommandEntryExecutionAction.cws" data COPYONLY)
configure_file("data/TwoCommandEntryWithResults.cws" data COPYONLY)

set(PATH_TO_TEST_NOTEBOOKS ${CMAKE_CURRENT_BINARY_DIR}/data)
configure_file (config-cantor-test.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantor-test.h )

add_executable( testworksheet ${worksheettest_SRCS})
#add_test(NAME testworksheet COMMAND testworksheet)
target_link_libraries( testworksheet
    cantorlibs
    cantor_config
    Qt6::Test
    Qt6::PrintSupport
    Qt6::Xml
    KF6::TextEditor
    Poppler::Qt6
    LibXslt::LibXslt
    LibXml2::LibXml2
)

if(LIBSPECTRE_FOUND)
    target_link_libraries(testworksheet ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)
if(Discount_FOUND)
    target_link_libraries(testworksheet Discount::Lib)
endif(Discount_FOUND)
