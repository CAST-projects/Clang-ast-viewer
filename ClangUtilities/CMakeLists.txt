cmake_minimum_required(VERSION 2.8.9)


include_directories("${THIRD_PARTY_INCLUDE_ROOT}${CLANG}")
include_directories("${THIRD_PARTY_INCLUDE_ROOT}${CLANG}\\x64")

set(ClangUtilities_Srcs 
	TemplateUtilities.cpp 
	StringLiteralExtractor.cpp
	)
	
set(ClangUtilities_Hdrs 
	TemplateUtilities.h 
	StringLiteralExtractor.h
	)

add_library(ClangUtilities STATIC ${ClangUtilities_Srcs} ${ClangUtilities_Hdrs})



# Probably some can be removed...
target_link_libraries (ClangUtilities 
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


