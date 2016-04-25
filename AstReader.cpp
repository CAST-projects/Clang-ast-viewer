#include "AstReader.h"
#include <sstream>
#include "CommandLineSplitter.h"
#include <iostream>


#pragma warning (push)
#pragma warning (disable:4100 4127 4800 4512 4245 4291 4510 4610 4324 4267 4244 4996)
#include <llvm/Support/Path.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/Decl.h>
#include <clang/Lex/Lexer.h>
#pragma warning (pop)

using namespace clang;

GenericAstNode::GenericAstNode() :
    myParent(nullptr)
{

}

int GenericAstNode::findChildIndex(GenericAstNode *node)
{
    auto it = std::find_if(myChidren.begin(), myChidren.end(), [node](std::unique_ptr<GenericAstNode> const & n){return n.get() == node; });
    return it == myChidren.end() ?
        -1 :
        it - myChidren.begin();
}

void GenericAstNode::attach(std::unique_ptr<GenericAstNode> child)
{
    child->myParent = this;
    myChidren.push_back(std::move(child));
}

struct SourceRangeVisitor : boost::static_visitor<SourceRange>
{
    template<class T>
    SourceRange operator()(T const *t) const
    {
        if (t == nullptr)
            return SourceRange();
        return t->getSourceRange();
    }
};

SourceRange GenericAstNode::getRange()
{
    return boost::apply_visitor(SourceRangeVisitor(), myAstNode);
}

bool GenericAstNode::getRangeInMainFile(std::pair<int, int> &result, clang::SourceManager const &manager, clang::ASTContext &context)
{
    auto range = getRange();
    if (range.isInvalid())
    {
        return false;
    }
    auto start = manager.getDecomposedSpellingLoc(range.getBegin());
    auto end = manager.getDecomposedSpellingLoc(clang::Lexer::getLocForEndOfToken(range.getEnd(), 0, manager, context.getLangOpts()));
    if (start.first != end.first || start.first != manager.getMainFileID())
    {
        //Not in the same file, or not in the main file (probably #included)
        return false;
    }
    result = std::make_pair(start.second, end.second);
    return true;
}


struct NodeColorVisitor : boost::static_visitor<int>
{
    int operator()(Decl const *) const
    {
        return 0;
    }
    int operator()(Stmt const *) const
    {
        return 1;
    }
};

int GenericAstNode::getColor()
{
    return boost::apply_visitor(NodeColorVisitor(), myAstNode);
}

class AstDumpVisitor : public RecursiveASTVisitor<AstDumpVisitor>
{
public:
    using PARENT = clang::RecursiveASTVisitor<AstDumpVisitor>;
    AstDumpVisitor(clang::CompilerInstance *CI, GenericAstNode *rootNode) :
        myRootNode(rootNode)
    {
        myStack.push_back(myRootNode);
    }

    std::string getTypeName(QualType qualType)
    {
        auto langOptions = clang::LangOptions{};
        auto printPolicy = PrintingPolicy{ langOptions };
        printPolicy.SuppressSpecifiers = false;
        printPolicy.ConstantArraySizeAsWritten = false;
        return qualType.getAsString(printPolicy);

    }
    std::string getFunctionPrototype(FunctionDecl *f)
    {
        std::ostringstream os;
        os << getTypeName(f->getReturnType()) << ' ' << f->getNameAsString() << '(';
        bool first = true;
        for (auto param : f->parameters())
        {
            if (!first)
            {
                os << ", ";
            }
            first = false;
            os << getTypeName(param->getType()) << ' ' << param->getNameAsString();
        }
        os << ')';
        return os.str();
    }


    bool TraverseDecl(clang::Decl *decl)
    {
        if (decl == nullptr)
        {
            return PARENT::TraverseDecl(decl);
        }
        auto node = std::make_unique<GenericAstNode>();
        node->myAstNode = decl;
        node->name = decl->getDeclKindName() + std::string("Decl"); // Try to mimick clang default dump
        if (auto *FD = dyn_cast<FunctionDecl>(decl))
        {
            node->name += " " + getFunctionPrototype(FD);
        }
        else if (auto *ND = dyn_cast<NamedDecl>(decl))
        {
            node->name += " " + ND->getNameAsString();
        }
        auto nodePtr = node.get();
        myStack.back()->attach(std::move(node));
        myStack.push_back(nodePtr);
        auto res = PARENT::TraverseDecl(decl);
        myStack.pop_back();
        return res;
    }

    bool TraverseStmt(clang::Stmt *stmt)
    {
        if (stmt == nullptr)
        {
            return PARENT::TraverseStmt(stmt);
        }
        auto node = std::make_unique<GenericAstNode>();
        node->myAstNode = stmt;
        node->name = stmt->getStmtClassName();
        auto nodePtr = node.get();
        myStack.back()->attach(std::move(node));
        myStack.push_back(nodePtr);
        auto res = PARENT::TraverseStmt(stmt);
        myStack.pop_back();
        return res;
    }

    bool VisitStringLiteral(clang::StringLiteral *s)
    {
        myStack.back()->name += (" " + s->getBytes()).str();
        return true;
    }

    bool TraverseType(clang::QualType type)
    {
        if (type.isNull())
        {
            return PARENT::TraverseType(type);
        }
        auto node = std::make_unique<GenericAstNode>();
        //node->myType = d;
        node->name = type->getTypeClassName();
        auto nodePtr = node.get();
        myStack.back()->attach(std::move(node));
        myStack.push_back(nodePtr);
        auto res = PARENT::TraverseType(type);
        myStack.pop_back();
        return res;
    }

private:
    std::vector<GenericAstNode*> myStack;
    GenericAstNode *myRootNode;
};

clang::SourceManager &AstReader::getManager()
{
    return myAst->getSourceManager();
}

clang::ASTContext &AstReader::getContext()
{
    return myAst->getASTContext();
}

GenericAstNode *AstReader::getRealRoot()
{
    return myArtificialRoot->myChidren.front().get();
}

GenericAstNode *AstReader::findPosInChildren(std::vector<std::unique_ptr<GenericAstNode>> const &candidates, int position)
{
    for (auto &candidate : candidates)
    {
        std::pair<int, int> location;
        if (!candidate->getRangeInMainFile(location, getManager(), getContext()))
        {
            continue;
        }
        if (location.first <= position && position <= location.second)
        {
            return candidate.get();
        }
    }
    return nullptr;
}

std::vector<GenericAstNode *> AstReader::getBestNodeMatchingPosition(int position)
{
    std::vector<GenericAstNode *> result;
    auto currentNode = getRealRoot();
    result.push_back(currentNode);
    currentNode = currentNode->myChidren[0].get();
    result.push_back(currentNode); // Translation unit does not have position
    while (true)
    {
        auto bestChild = findPosInChildren(currentNode->myChidren, position);
        if (bestChild == nullptr)
        {
            return result;
        }
        result.push_back(bestChild);
        currentNode = bestChild;
    }
}

GenericAstNode *AstReader::readAst(std::string const &sourceCode, std::string const &options)
{
    mySourceCode = sourceCode;
    myArtificialRoot = std::make_unique<GenericAstNode>();
    auto root = std::make_unique<GenericAstNode>();
    root->name = "AST";
    myArtificialRoot->attach(std::move(root));

    auto args = splitCommandLine(options);

    std::cout << "Launching Clang to create AST" << std::endl;
    myAst = clang::tooling::buildASTFromCodeWithArgs(mySourceCode, args);
    for (auto it = myAst->top_level_begin(); it != myAst->top_level_end(); ++it)
    {
        (*it)->dumpColor();
    }
    std::cout << "Visiting AST and creating Qt Tree" << std::endl;
    auto visitor = AstDumpVisitor{nullptr, getRealRoot()};
    visitor.TraverseDecl(myAst->getASTContext().getTranslationUnitDecl());
    return myArtificialRoot.get();
}