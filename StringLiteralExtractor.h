#pragma once

#include <vector>
#pragma warning (push)
#pragma warning (disable:4100 4127 4800 4512 4245 4291 4510 4610 4324 4267 4244 4996)
#include <clang/AST/Decl.h>
#include <clang/Lex/Lexer.h>
#include <clang/Frontend/FrontendActions.h>
#pragma warning (pop)

std::vector<std::string>
splitStringLiteral(clang::StringLiteral *S, const clang::SourceManager &SM, const clang::LangOptions &Features, const clang::TargetInfo &Target);


