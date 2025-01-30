#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filetraversethread.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void warning(QWidget *parent, const char *msg)
{
    QMessageBox::warning(parent, QObject::tr("warning"), QObject::tr(msg));
}

void critical(QWidget *parent, const char *msg)
{
    QMessageBox::critical(parent, QObject::tr("critical"), QObject::tr(msg));
}

void MainWindow::on_pushButton_folderSelect_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty()) {
        ui->lineEdit_folderSelect->setText(directory);
    } else {
        warning(this, "No directory was selected");
    }
}

void MainWindow::on_searchButton_clicked()
{
    QString input = ui->searchEdit->text();
    QString folder = ui->lineEdit_folderSelect->text();
    // if (!input.isEmpty() && !folder.isEmpty()) {
    if (!folder.isEmpty()) {

        FileTraverseThread *t = new FileTraverseThread(folder, this);
        connect(t, &FileTraverseThread::updateProgress, this, &MainWindow::updateProgress);
        connect(t, &FileTraverseThread::updateFileInfo, this, &MainWindow::updateFileInfo);

        t->start();
    }
}

/* Search Thread Functions */

void MainWindow::updateProgress(QString file)
{
    QString msg = tr("Seaching: ");
    msg += file;
    msg += " ...";
    ui->labelStatus->setText(msg);
}

void MainWindow::updateFileInfo(QStandardItemModel *model)
{
    ui->tableView_results->setModel(model);
    // ui->tableView_results->setSortingEnabled(true); // will cause index out of length error
    ui->tableView_results->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->tableView_results->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_results->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // ui label done
    ui->labelStatus->setText(tr("Done!"));
}
