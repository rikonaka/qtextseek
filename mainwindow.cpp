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
#include <QTableView>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    searchThread = nullptr;
    fileWorker = nullptr;
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    stopSearchThread();
    delete ui;
}

void information(QWidget *parent, const char *msg)
{
    QMessageBox::information(parent, QObject::tr("information"), QObject::tr(msg));
}

void warning(QWidget *parent, const char *msg)
{
    QMessageBox::warning(parent, QObject::tr("warning"), QObject::tr(msg));
}

void critical(QWidget *parent, const char *msg)
{
    QMessageBox::critical(parent, QObject::tr("critical"), QObject::tr(msg));
}

void MainWindow::stopSearchThread()
{
    if (fileWorker)
        fileWorker->stopRequested = true;
    if (searchThread)
    {
        if (searchThread->isRunning())
        {
            searchThread->quit();
            searchThread->wait();
        }
    }
    if (fileWorker)
    {
        delete fileWorker;
        fileWorker = nullptr;

    }
    if (searchThread)
    {
        delete searchThread;
        searchThread = nullptr;
    }
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

QList<QString> splitExtension(QString *ext)
{
    QList<QString> ret;
    QStringList split = ext->split(",");
    for (QString &s: split)
    {
        ret.append(s.trimmed());
    }
    return ret;
}

void MainWindow::on_searchButton_clicked()
{
    QString input = ui->searchEdit->text();
    QString folder = ui->lineEdit_folderSelect->text();
    if (input.isEmpty())
    {
        warning(this, "Please input what your wanna search!");
    }
    else
    {
        if (folder.isEmpty())
        {
            warning(this, "Please select the target folder!");
        }
        else
        {
            QString ext = ui->lineEdit_fileExtension->text();
            QList<QString> extensions = splitExtension(&ext);
            FileTraverseWorker *worker = new FileTraverseWorker(folder, input, extensions);

            connect(worker, &FileTraverseWorker::updateProgress, this, &MainWindow::updateProgressFunc);
            connect(worker, &FileTraverseWorker::updateFileInfo, this, &MainWindow::updateFileInfoFunc);

            QThread *workerThread = new QThread();
            worker->moveToThread(workerThread);

            connect(workerThread, &QThread::started, worker, &FileTraverseWorker::run);
            connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

            workerThread->start();
            searchThread = workerThread;

            if (fileWorker)
                delete fileWorker;
            fileWorker = worker;
        }
    }
}

/* Search Thread Functions */

void MainWindow::updateProgressFunc(QString file, bool skip)
{
    QString msg;
    if (skip)
        msg += tr("Skip: ");
    else
        msg += tr("Seaching: ");
    msg += file;
    msg += " ...";
    ui->labelStatus->setText(msg);
}

void MainWindow::updateFileInfoFunc(QStandardItemModel *model)
{
    ui->tableView_results->setModel(model);
    // ui->tableView_results->setSortingEnabled(true); // will cause index out of length error
    ui->tableView_results->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->tableView_results->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_results->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView_results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    //ui->tableView_results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableView_results->setColumnHidden(3, true); // hide column 3 and show it in right widgt

    // ui label done
    if (fileWorker->stopRequested)
        ui->labelStatus->setText(tr("Stoped!"));
    else
        ui->labelStatus->setText(tr("Done!"));


    connect(ui->tableView_results, &QTableView::clicked, this, &MainWindow::tableRowClicked);
}

void MainWindow::tableRowClicked(const QModelIndex &index)
{
    int row = index.row();
    // int col = index.column();
    QAbstractItemModel *model = ui->tableView_results->model();
    QModelIndex v = model->index(row, 3);
    ui->textEdit_results->setText(v.data().toString());
}

void MainWindow::on_stopButton_clicked()
{
    //qDebug() << "stop";
    stopSearchThread();
    ui->labelStatus->setText(tr("Stoped!"));
}

