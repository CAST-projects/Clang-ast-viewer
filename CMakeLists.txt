cmake_minimum_required(VERSION 2.8.9)
project(ClangAstViewer)

set (CMAKE_PREFIX_PATH "D:\\Qt\\5.5\\msvc2013_64")
set (THIRD_PARTY_ROOT "D:\\Dev\\Trunk\\3rdParties\\cpp\\")
set (THIRD_PARTY_INCLUDE_ROOT "${THIRD_PARTY_ROOT}includes\\")
set (CLANG "llvm-3.7.1\\")
set (BOOST "boost-1_55_0\\")
#set (CLANG_PREFIX_PATH ${THIRD_PARTY_ROOT}\\binaries_$(ConfigurationName)_x64\\${CLANG}\\)
set (CLANG_PREFIX_PATH C:\\Users\\LJO\\.conan\\data\\llvm\\3.9.0.3\\yle\\stable\\package\\2c0843cc59ff2d07e33c808b3398bc624e6b54e4\\lib\\)

add_subdirectory(ClangUtilities)


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

include_directories("C:\\Users\\LJO\\.conan\\data\\llvm\\3.9.0.3\\yle\\stable\\package\\2c0843cc59ff2d07e33c808b3398bc624e6b54e4\\include")
# include_directories("${THIRD_PARTY_INCLUDE_ROOT}${CLANG}\\x64")
include_directories("${THIRD_PARTY_INCLUDE_ROOT}${BOOST}")


set(ClangAst_Forms MainWindow.ui)
set(ClangAst_Srcs 
	main.cpp 
	MainWindow.cpp 
	Highlighter.cpp
	AstReader.cpp
	AstModel.cpp
	CommandLineSplitter.cpp
	)

set(ClangAst_Hdrs 
	MainWindow.h 
	Highlighter.h
	AstReader.h
	AstModel.h
	CommandLineSplitter.h
	)

QT5_WRAP_UI(UIS_HDRS ${ClangAst_Forms})

# Tell CMake to create the helloworld executable
add_executable(ClangAstViewer ${ClangAst_Srcs} ${ClangAst_Hdrs} ${UIS_HDRS})


# Probably some can be removed...
target_link_libraries (ClangAstViewer 
	ClangUtilities
	${CLANG_PREFIX_PATH}clangAnalysis.lib
	${CLANG_PREFIX_PATH}clangAST.lib
	${CLANG_PREFIX_PATH}clangASTMatchers.lib
	${CLANG_PREFIX_PATH}clangBasic.lib
	${CLANG_PREFIX_PATH}clangDriver.lib
	${CLANG_PREFIX_PATH}clangEdit.lib
	${CLANG_PREFIX_PATH}clangFrontend.lib
	${CLANG_PREFIX_PATH}clangLex.lib
	${CLANG_PREFIX_PATH}clangParse.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerCore.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerCheckers.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerFrontend.lib
	${CLANG_PREFIX_PATH}clangRewrite.lib
	${CLANG_PREFIX_PATH}clangSema.lib
	${CLANG_PREFIX_PATH}clangSerialization.lib
	${CLANG_PREFIX_PATH}clangTooling.lib
	${CLANG_PREFIX_PATH}LLVMBitReader.lib
	${CLANG_PREFIX_PATH}LLVMMC.lib
	${CLANG_PREFIX_PATH}LLVMMCParser.lib
	${CLANG_PREFIX_PATH}LLVMOption.lib
	${CLANG_PREFIX_PATH}LLVMSupport.lib
	${CLANG_PREFIX_PATH}LLVMProfileData.lib
	${CLANG_PREFIX_PATH}LLVMCore.lib
	version.lib
	)

# Use the Widgets module from Qt 5.
qt5_use_modules(ClangAstViewer Widgets)


