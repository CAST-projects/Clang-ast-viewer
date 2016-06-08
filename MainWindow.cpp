#include "MainWindow.h"
#include <qmessagebox.h>
#include <qfilesystemmodel.h>
#include <qstringlist.h>
#include "AstModel.h"



MainWindow::MainWindow(QWidget *parent) : 
    QMainWindow(parent)
{
    myUi.setupUi(this);

    connect(myUi.actionRefresh, &QAction::triggered, this, &MainWindow::RefreshAst);

    myHighlighter = new Highlighter(myUi.codeViewer->document());
    myUi.nodeProperties->setHeaderLabels({ "Property", "Value" });
}

void MainWindow::RefreshAst()
{
    auto ast = myReader.readAst(myUi.codeViewer->document()->toPlainText().toStdString(),
        myUi.commandLineArgs->document()->toPlainText().toStdString());
    auto model = new AstModel(std::move(ast));

    myUi.astTreeView->setModel(model);
    myUi.astTreeView->setRootIndex(model->rootIndex());
    connect(myUi.astTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &MainWindow::HighlightCodeMatchingNode);
    connect(myUi.astTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &MainWindow::DisplayNodeProperties);
    connect(myUi.codeViewer, &QTextEdit::cursorPositionChanged, this, &MainWindow::HighlightNodeMatchingCode);
}

void MainWindow::HighlightCodeMatchingNode(const QModelIndex &newNode, const QModelIndex &previousNode)
{
    auto node = myUi.astTreeView->model()->data(newNode, Qt::NodeRole).value<GenericAstNode*>();
    auto &manager = myReader.getManager();
    std::pair<int, int> location;
    if (!node->getRangeInMainFile(location, manager, myReader.getContext()))
    {
        return;
    }
    auto cursor = myUi.codeViewer->textCursor();
    cursor.setPosition(location.first);
    cursor.setPosition(location.second, QTextCursor::KeepAnchor);
    myUi.codeViewer->setTextCursor(cursor);
}

void MainWindow::DisplayNodeProperties(const QModelIndex &newNode, const QModelIndex &previousNode)
{
    myUi.nodeProperties->clear();
    auto node = myUi.astTreeView->model()->data(newNode, Qt::NodeRole).value<GenericAstNode*>();
    for (auto &prop : node->getProperties())
    {
        new QTreeWidgetItem(myUi.nodeProperties, QStringList{ QString::fromStdString(prop.first), QString::fromStdString(prop.second) });
    }
}

void MainWindow::HighlightNodeMatchingCode()
{
    auto cursorPosition = myUi.codeViewer->textCursor().position();
    auto nodePath = myReader.getBestNodeMatchingPosition(cursorPosition);
    auto model = myUi.astTreeView->model();
    if (!nodePath.empty())
    {
        auto currentIndex = model->index(0, 0); // Returns the root
        currentIndex = model->index(0, 0, currentIndex); // Returns the AST node
        auto currentNode = nodePath.front();
        bool first = true;
        for (auto node : nodePath)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                auto index = currentNode->findChildIndex(node);
                if (index == -1)
                {
                    // Something wrong, just silently return
                    return;
                }
                currentIndex = model->index(index, 0, currentIndex);
                currentNode = node;
            }
        }
        myUi.astTreeView->scrollTo(currentIndex, QAbstractItemView::EnsureVisible);
        auto selectionModel = myUi.astTreeView->selectionModel();
        selectionModel->select(currentIndex, QItemSelectionModel::ClearAndSelect);
        DisplayNodeProperties(currentIndex, currentIndex); // Since we won't use the previous node, it's not an issue if it is wrong...
    }
}


