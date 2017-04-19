cmake_minimum_required(VERSION 3.2)

project(ClangAstViewer CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# to be set when calling cmake (e.g. -DLLVM_INSTALL_DIR=...)
# path is where there is include/ and lib/ of LLVM/clang
set(LLVM_INSTALL_DIR "../llvm-install-dir" CACHE  STRING "")

set(LLVM_DIR "${LLVM_INSTALL_DIR}/lib/cmake/llvm")
find_package(LLVM REQUIRED
    NO_CMAKE_PATH
    NO_DEFAULT_PATH)

# where to get Clang's exported cmake-file for importing
set(Clang_DIR "${LLVM_INSTALL_DIR}/lib/cmake/clang")
find_package(Clang REQUIRED
    NO_CMAKE_PATH
    NO_DEFAULT_PATH)

# ClangUtilities library
add_library(ClangUtilities
    ClangUtilities/TemplateUtilities.cpp
    ClangUtilities/StringLiteralExtractor.cpp
    ClangUtilities/TemplateUtilities.h
    ClangUtilities/StringLiteralExtractor.h)

# unfortunately LLVM CMake exports do not contain the include_directories,
# so it needs to be added "manually" here

target_include_directories(ClangUtilities PUBLIC ${LLVM_INSTALL_DIR}/include)
target_link_libraries(ClangUtilities
    PRIVATE
        clangAnalysis
        clangAST
        clangBasic
        clangFrontend
        clangIndex
        clangLex
        clangTooling
        LLVMSupport)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# UIs
set(ClangAst_Forms MainWindow.ui)

# Srcs
set(ClangAst_Srcs
	main.cpp
	MainWindow.cpp
	Highlighter.cpp
	AstReader.cpp
	AstModel.cpp
	CommandLineSplitter.cpp)

# Headers for MOC and vcproj-file
set(ClangAst_Hdrs
	MainWindow.h
	Highlighter.h
	AstReader.h
	AstModel.h
	CommandLineSplitter.h)

QT5_WRAP_UI(UIS_HDRS ${ClangAst_Forms})

# Tell CMake to create the ClangAstViewer executable
add_executable(ClangAstViewer ${ClangAst_Srcs} ${ClangAst_Hdrs} ${UIS_HDRS})
target_link_libraries(ClangAstViewer ClangUtilities)

# Use the Widgets module from Qt 5.
qt5_use_modules(ClangAstViewer Widgets)
