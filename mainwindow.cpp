#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filetraversethread.h"

#include <QFileDialog>
#include <QMessageBox>
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
    if (searchThread) {
        if (searchThread->isRunning()) {
            searchThread->quit();
            searchThread->wait();
        }
    }
    if (fileWorker) {
        delete fileWorker;
        fileWorker = nullptr;

    }
    if (searchThread) {
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
    for (QString &s: split) {
        ret.append(s.trimmed());
    }
    return ret;
}

void MainWindow::on_searchButton_clicked()
{
    stopSearchThread();

    QString input = ui->searchEdit->text();
    QString folder = ui->lineEdit_folderSelect->text();
    if (input.isEmpty()) {
        warning(this, "Please input what your wanna search!");
    }
    else {
        if (folder.isEmpty()) {
            warning(this, "Please select the target folder!");
        }
        else {
            QString ext = ui->lineEdit_fileExtension->text();
            QList<QString> extensions = splitExtension(&ext);
            FileTraverseWorker *worker = new FileTraverseWorker(folder, input, extensions);

            connect(worker, &FileTraverseWorker::updateProgress, this, &MainWindow::updateProgressFunc);
            connect(worker, &FileTraverseWorker::updateProgressStoped, this, &MainWindow::updateProgressStopedFunc);
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

void MainWindow::updateProgressFunc(QString file, int status)
{
    /* 0 => Searching
     * 1 => Skip
     * 2 => Notfound
     */
    QString msg;
    if (status == 0)
        msg += tr("Seaching: ");
    else if (status == 1)
        msg += tr("Skip: ");
    else // == 2
        msg += tr("Notfound: ");
    msg += file;
    msg += " ...";
    ui->labelStatus->setText(msg);
}

void MainWindow::updateProgressStopedFunc()
{
    ui->labelStatus->setText(tr("Stoped!"));
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
    ui->tableView_results->setColumnHidden(3, true); // target column

    // ui label done
    if (fileWorker->stopRequested)
        ui->labelStatus->setText(tr("Stoped!"));
    else
        ui->labelStatus->setText(tr("Done!"));


    connect(ui->tableView_results, &QTableView::clicked, this, &MainWindow::tableRowClicked);
}

struct fileContentPositions {
    QString content;
    QList<qsizetype> positions;
};

fileContentPositions *contentRegex(QString &filePath, QString &target)
{
    /* start read content from file */
    QString content;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString msg = "Can not open file: ";
        msg += filePath;
        content += msg;
    } else {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();

            QString filteredLine;
            for (QChar &c : line) {
                if (c.isPrint()) {
                    filteredLine.append(c);
                }
            }
            content.append(filteredLine + "\n");
        }
    }
    file.close();
    /* end read content from file */

    // regex work
    QRegularExpression re(target);
    // QRegularExpressionMatch match = re.match(content);
    QRegularExpressionMatchIterator i = re.globalMatch(content);

    fileContentPositions *ret = new fileContentPositions;
    ret->content = content;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        qsizetype start = match.capturedStart();
        qDebug() << "Found match at position: " << start;
        qDebug() << "Matched text: " << match.captured(0);
        ret->positions.append(start);
    }

    return ret;
}

void MainWindow::tableRowClicked(const QModelIndex &index)
{
    int row = index.row();
    // int col = index.column();
    QAbstractItemModel *model = ui->tableView_results->model();
    QModelIndex pp = model->index(row, 2);
    QModelIndex tt = model->index(row, 3);
    QString filePath = pp.data().toString();
    QString target = tt.data().toString();

    fileContentPositions *fcp = contentRegex(filePath, target);
    ui->textEdit_results->setText(fcp->content);

    QTextCursor cursor = ui->textEdit_results->textCursor();
    // clear the format
    cursor.select(QTextCursor::Document);
    QTextCharFormat resetFormat;
    cursor.mergeCharFormat(resetFormat);

    for (qsizetype &p: fcp->positions) {
        qDebug() << "p: " << p;
        cursor.setPosition(p);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, target.length());
        QTextCharFormat format;
        format.setBackground(QColor("yellow"));
        cursor.mergeCharFormat(format);
    }
    ui->textEdit_results->setTextCursor(cursor);
    ui->textEdit_results->ensureCursorVisible();

    delete fcp;
}

void MainWindow::on_stopButton_clicked()
{
    qDebug() << "stop button clicked";
    stopSearchThread();
}

