#include "AstReader.h"
#include <sstream>
#include "CommandLineSplitter.h"
#include <iostream>
#include "ClangUtilities/StringLiteralExtractor.h"
#include "ClangUtilities/TemplateUtilities.h"


#pragma warning (push)
#pragma warning (disable:4100 4127 4800 4512 4245 4291 4510 4610 4324 4267 4244 4996)
#include <llvm/Support/Path.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/Decl.h>
#include <clang/Lex/Lexer.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Mangle.h>
#pragma warning (pop)

using namespace clang;

namespace props
{
    std::string const Name = "Name";
    std::string const Mangling = "Mangling";
    std::string const Referenced = "Referenced name";
    std::string const Resolved = "Resolved name";
    std::string const Value = "Value";
    std::string const InterpretedValue = "Interpreted value";
}

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


void GenericAstNode::setProperty(std::string const &propertyName, std::string const &value)
{
    myProperties[propertyName] = value;
}

GenericAstNode::Properties const &GenericAstNode::getProperties() const
{
    return myProperties;
}




class AstDumpVisitor : public RecursiveASTVisitor<AstDumpVisitor>
{
public:
    using PARENT = clang::RecursiveASTVisitor<AstDumpVisitor>;
    AstDumpVisitor(clang::ASTContext &context, GenericAstNode *rootNode) :
        myRootNode(rootNode),
        myAstContext(context)
    {
        myStack.push_back(myRootNode);
    }

    bool shouldVisitTemplateInstantiations()
    {
        return true;
    }




    std::string getMangling(clang::NamedDecl *ND)
    {
        if (auto funcContext = dyn_cast<FunctionDecl>(ND->getDeclContext()))
        {
            if (funcContext->getTemplatedKind() == FunctionDecl::TK_FunctionTemplate)
            {
                return "<Cannot mangle name inside a template>";
            }
        }
        else if (auto recContext = dyn_cast<CXXRecordDecl>(ND->getDeclContext()))
        {
            if (recContext->getDescribedClassTemplate() != nullptr)
            {
                return "<Cannot mangle name inside a template>";
            }
        }
        auto mangleContext = ND->getASTContext().createMangleContext();
        std::string FrontendBuf;
        llvm::raw_string_ostream FrontendBufOS(FrontendBuf);
        if (mangleContext->shouldMangleDeclName(ND) && !isa<CXXConstructorDecl>(ND) && !isa<CXXDestructorDecl>(ND) && !isa<ParmVarDecl>(ND))
        {
            mangleContext->mangleName(ND, FrontendBufOS);
            return FrontendBufOS.str();
        }
        else
        {
            return "<No Mangling>";
        }
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
            node->name += " " + clang_utilities::getFunctionPrototype(FD, false);
            if (FD->getTemplatedKind() != FunctionDecl::TK_FunctionTemplate)
            {
                node->setProperty(props::Mangling, getMangling(FD));
            }
            node->setProperty(props::Name, clang_utilities::getFunctionPrototype(FD, true));
        }
        else if (auto *PVD = dyn_cast<ParmVarDecl>(decl))
        {
            if (auto *PFD = dyn_cast<FunctionDecl>(decl->getParentFunctionOrMethod()))
            {
                if (PFD->getTemplatedKind() != FunctionDecl::TK_FunctionTemplate)
                {
                    node->setProperty(props::Mangling, getMangling(PFD));
                }
            }
            else
            {
                node->setProperty(props::Mangling, getMangling(PVD));
            }
            node->setProperty(props::Name, PVD->getNameAsString());
        }
        else if (auto *VD = dyn_cast<VarDecl>(decl))
        {
            //node->setProperty(props::Mangling, getMangling(VD));
            node->setProperty(props::Name, VD->getNameAsString());
        }
        else if (auto *ND = dyn_cast<NamedDecl>(decl))
        {
            node->name += " " + ND->getNameAsString();
            node->setProperty(props::Name, ND->getNameAsString());
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
        myStack.back()->setProperty(props::InterpretedValue, s->getBytes());
        auto parts = clang_utilities::splitStringLiteral(s, myAstContext.getSourceManager(), myAstContext.getLangOpts(), myAstContext.getTargetInfo());
        if (parts.size() == 1)
        {
            myStack.back()->setProperty(props::Value, parts[0]);

        }
        else
        {
            int i = 0;
            for (auto &part : parts)
            {
                ++i;
                myStack.back()->setProperty(props::Value + " " + std::to_string(i), part);

            }
        }
        return true;
    }

    bool VisitIntegerLiteral(clang::IntegerLiteral *i)
    {
        bool isSigned = i->getType()->isSignedIntegerType();
        myStack.back()->setProperty(props::Value, i->getValue().toString(10, isSigned));
        return true;
    }

    bool VisitCharacterLiteral(clang::CharacterLiteral *c)
    {
        myStack.back()->setProperty(props::Value, std::string(1, c->getValue()));
        return true;
    }

    bool VisitFloatingLiteral(clang::FloatingLiteral *f)
    {
        myStack.back()->setProperty(props::Value, std::to_string(f->getValueAsApproximateDouble()));
        return true;
    }

    void addReference(GenericAstNode *node, clang::NamedDecl *referenced, std::string const &label)
    {
        auto funcDecl = dyn_cast<FunctionDecl>(referenced);
        myStack.back()->setProperty(label, funcDecl == nullptr ?
            referenced->getNameAsString() :
            clang_utilities::getFunctionPrototype(funcDecl, false));
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr *ref)
    {
        addReference(myStack.back(), ref->getDecl(), props::Referenced);
        addReference(myStack.back(), ref->getFoundDecl(), props::Resolved);

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
    ASTContext &myAstContext;
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
    auto visitor = AstDumpVisitor{ myAst->getASTContext(), getRealRoot() };
    visitor.TraverseDecl(myAst->getASTContext().getTranslationUnitDecl());
    return myArtificialRoot.get();
}