#include "mainwindow.h"
#include "ui_mainwindow.h"

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

QList<QFileInfo> getFilesRecursively(const QString &dirPath) {
    QDir dir(dirPath);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    QList<QFileInfo> ret;

    for (QFileInfo &fileInfo : list) {
        if (fileInfo.isDir()) {
            qDebug() << "Directory: " << fileInfo.absoluteFilePath();
            getFilesRecursively(fileInfo.absoluteFilePath());
        } else {
            // qDebug() << "File:" << fileInfo.absoluteFilePath()
            //          << "Size:" << fileInfo.size() << "bytes"
            //          << "Last Modified:" << fileInfo.lastModified();
            if (fileInfo.isFile())
            {
                ret.push_back(fileInfo);
            }
        }
    }
    return ret;
}

class FileInfoModel : public QStandardItemModel {
public:
    FileInfoModel(const QString &dirPath) {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        for (QFileInfo &fileInfo : files) {
            if (fileInfo.isDir()) {
                getFilesRecursively(fileInfo.absoluteFilePath());
            } else {
                if (fileInfo.isFile())
                {
                    addItem(fileInfo);
                }
            }
        }
    }

private:
    void addItem(const QFileInfo& fileInfo) {
        QString fileName = fileInfo.fileName();
        QString fileSize = QString::number(fileInfo.size()) + " bytes";
        QString filePath = fileInfo.absoluteFilePath();
        QString fileContent = getFileContent(fileInfo.absoluteFilePath());

        QString displayText = fileName + "|" + fileSize + "\n" + filePath + "\n" + fileContent;
        QStandardItem *item = new QStandardItem(displayText);
        appendRow(item);

    }

    QString getFileContent(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString msg = "Can not open file: ";
            msg += filePath;
            return msg;
        }

        QTextStream in(&file);
        QString content = in.read(100);
        return content.isEmpty() ? "Empty file!" : content;
    }
};

class FileInfoDelegate : public QStyledItemDelegate {
public:
    FileInfoDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QString text = index.data().toString();

        painter->save();
        // painter->setPen(option.state & QStyle::State_Selected ? Qt::white : Qt::blue);

        QRect rect = option.rect;
        int x = rect.left();
        int y = rect.top();

        // 绘制文件名（蓝色，较大字体）
        QString fileName = text.section("|", 0, 0);  // 获取文件名
        painter->setFont(QFont("Arial", 12, QFont::Bold));  // 设置较大字体
        painter->setPen(QColor(0, 0, 255));  // 蓝色
        painter->drawText(x, y, fileName);
        x += painter->fontMetrics().horizontalAdvance(fileName);

        // 文件大小（灰色字体）
        QString fileSize = text.section("|", 1, 1);  // 获取文件大小
        painter->setFont(QFont("Arial", 10));
        painter->setPen(QColor(169, 169, 169));  // 灰色
        painter->drawText(x, y, " | " + fileSize);
        x += painter->fontMetrics().horizontalAdvance(" | " + fileSize);

        // 文件路径（绿色字体）
        QString filePath = text.section("\n", 0, 0).section("|", 2, 2);  // 获取文件路径
        painter->setFont(QFont("Arial", 10));
        painter->setPen(QColor(0, 128, 0));  // 绿色
        painter->drawText(x, y, "\n" + filePath);
        x += painter->fontMetrics().horizontalAdvance("\n" + filePath);

        // 文件内容（灰色字体）
        QString fileContent = text.section("\n", 1, 1);  // 获取文件内容
        painter->setFont(QFont("Arial", 10));
        painter->setPen(QColor(169, 169, 169));  // 灰色
        painter->drawText(x, y, "\n" + fileContent);

        painter->restore();
    }
};

void MainWindow::on_searchButton_clicked()
{
    QString input = ui->searchEdit->text();
    QString folder = ui->lineEdit_folderSelect->text();
    if (!input.isEmpty() && !folder.isEmpty()) {
        FileInfoModel* model = new FileInfoModel(folder);
        ui->listView_results->setModel(model);
        FileInfoDelegate* delegate = new FileInfoDelegate(ui->listView_results);
        ui->listView_results->setItemDelegate(delegate);
    }
}

