cmake_minimum_required(VERSION 2.8.9)
project(ClangAstViewer)

set (CMAKE_PREFIX_PATH "D:\\Qt\\5.5\\msvc2013_64")
set (CLANG_ROOT "C:\\Dev\\Trunk\\3rdParties\\cpp\\binaries\\llvm-3.7.1\\")
set (BOOST_ROOT "C:\\Dev\\Trunk\\3rdParties\\cpp\\binaries\\boost-1_55_0\\")
set (CLANG_PREFIX_PATH ${CLANG_ROOT}x64\\release\\)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

include_directories("${CLANG_ROOT}include")
include_directories("${CLANG_ROOT}include\\x64")
include_directories("${BOOST_ROOT}include")


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
	main.cpp 
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
	${CLANG_PREFIX_PATH}clangAnalysis.lib
	${CLANG_PREFIX_PATH}clangARCMigrate.lib
	${CLANG_PREFIX_PATH}clangAST.lib
	${CLANG_PREFIX_PATH}clangASTMatchers.lib
	${CLANG_PREFIX_PATH}clangBasic.lib
	${CLANG_PREFIX_PATH}clangCodeGen.lib
	${CLANG_PREFIX_PATH}clangDriver.lib
	${CLANG_PREFIX_PATH}clangDynamicASTMatchers.lib
	${CLANG_PREFIX_PATH}clangEdit.lib
	${CLANG_PREFIX_PATH}clangFormat.lib
	${CLANG_PREFIX_PATH}clangFrontend.lib
	${CLANG_PREFIX_PATH}clangFrontendTool.lib
	${CLANG_PREFIX_PATH}clangIndex.lib
	${CLANG_PREFIX_PATH}clangLex.lib
	${CLANG_PREFIX_PATH}clangParse.lib
	${CLANG_PREFIX_PATH}clangRewrite.lib
	${CLANG_PREFIX_PATH}clangRewriteFrontend.lib
	${CLANG_PREFIX_PATH}clangSema.lib
	${CLANG_PREFIX_PATH}clangSerialization.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerCheckers.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerCore.lib
	${CLANG_PREFIX_PATH}clangStaticAnalyzerFrontend.lib
	${CLANG_PREFIX_PATH}clangTooling.lib
	${CLANG_PREFIX_PATH}clangToolingCore.lib
	${CLANG_PREFIX_PATH}LLVMBitReader.lib
	${CLANG_PREFIX_PATH}LLVMBitWriter.lib
	${CLANG_PREFIX_PATH}LLVMCore.lib
	${CLANG_PREFIX_PATH}LLVMCppBackendCodeGen.lib
	${CLANG_PREFIX_PATH}LLVMCppBackendInfo.lib
	${CLANG_PREFIX_PATH}LLVMInstrumentation.lib
	${CLANG_PREFIX_PATH}LLVMInterpreter.lib
	${CLANG_PREFIX_PATH}LLVMMC.lib
	${CLANG_PREFIX_PATH}LLVMMCParser.lib
	${CLANG_PREFIX_PATH}LLVMObject.lib
	${CLANG_PREFIX_PATH}LLVMOption.lib
	${CLANG_PREFIX_PATH}LLVMSupport.lib
	${CLANG_PREFIX_PATH}LLVMXCoreAsmPrinter.lib
	${CLANG_PREFIX_PATH}LLVMXCoreCodeGen.lib
	${CLANG_PREFIX_PATH}LLVMXCoreDesc.lib
	${CLANG_PREFIX_PATH}LLVMXCoreDisassembler.lib
	${CLANG_PREFIX_PATH}LLVMXCoreInfo.lib
	)

# Use the Widgets module from Qt 5.
qt5_use_modules(ClangAstViewer Widgets)


