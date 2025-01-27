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

class FileInfoModel : public QStandardItemModel {
public:
    void AddItem(const QFileInfo& fileInfo) {
        QString fileName = fileInfo.fileName();
        QString fileSize = QString::number(fileInfo.size()) + " bytes";
        QString filePath = fileInfo.absoluteFilePath();
        QString fileContent = FileInfoModel::checkFileContent(filePath);

        QList<QStandardItem *> newRow;
        newRow.append(new QStandardItem(fileName));
        newRow.append(new QStandardItem(fileSize));
        newRow.append(new QStandardItem(filePath));
        newRow.append(new QStandardItem(fileContent));
        appendRow(newRow);
    }
    QString checkFileContent(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString msg = "Can not open file: ";
            msg += filePath;
            return msg;
        }

        QTextStream in(&file);
        QString content = in.read(10);
        return content.isEmpty() ? "Empty file!" : content;
    }
};

void MainWindow::on_searchButton_clicked()
{
    QString input = ui->searchEdit->text();
    QString folder = ui->lineEdit_folderSelect->text();
    // if (!input.isEmpty() && !folder.isEmpty()) {
    if (!folder.isEmpty()) {

        FileTraverseThread *t = new FileTraverseThread(folder, this);
        connect(t, &FileTraverseThread::progressUpdatedTotal, this, &MainWindow::updateProgressTotal);
        connect(t, &FileTraverseThread::progressUpdated, this, &MainWindow::updateProgress);
        connect(t, &FileTraverseThread::finished, this, &MainWindow::onFinished);

        t->start();

        FileInfoModel* model = new FileInfoModel(folder);

        model->setHorizontalHeaderLabels({tr("name"), tr("size"), tr("path"), tr("content")});
        ui->tableView_results->setModel(model);
        ui->tableView_results->setSortingEnabled(true);
        ui->tableView_results->setEditTriggers(QAbstractItemView::DoubleClicked);
        ui->tableView_results->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView_results->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableView_results->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->tableView_results->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->tableView_results->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        ui->tableView_results->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    }
}

void MainWindow::updateProgressTotal(int value)
{
    ui->progressBar->setRange(0, value);
}

void MainWindow::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::onFinished()
{
    ui->progressBar->deleteLater();
}

void MainWindow::updateFileInfo(QFileInfo *fileInfo)
{
    ui->tableView_results->addItem(QString("File: %1, Size: %2 bytes").arg(fileName).arg(fileSize));
}

