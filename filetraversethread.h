#ifndef FILETRAVERSETHREAD_H
#define FILETRAVERSETHREAD_H

#include <QThread>
#include <QString>
#include <QDir>
#include <QFileInfoList>
#include <QStandardItemModel>

extern struct MyFileInfo {
    QString fileName;
    qint64 fileSize;
    QString filePath;
    QString fileContent;
} my_file_info;

class FileTraverseThread : public QThread
{
    Q_OBJECT

public:
    explicit FileTraverseThread(const QString &dirPath, QObject *parent = nullptr)
        : QThread(parent), dirPath(dirPath) {}

signals:
    void updateProgress(QString file);
    void updateFileInfo(QStandardItemModel *model);

protected:
    void run() override {
        const QList<MyFileInfo> cfi = FileTraverseThread::traverse(dirPath);
        qDebug() << "files length: " << cfi.length();

        QStandardItemModel *model = new QStandardItemModel;
        model->setHorizontalHeaderLabels({tr("name"), tr("size"), tr("path"), tr("content")});
        emit updateFileInfo(model);

        for (const MyFileInfo &fileInfo : cfi) {
            QString fileName = fileInfo.fileName;
            QString fileSize = QString::number(fileInfo.fileSize) + " bytes";
            QString filePath = fileInfo.filePath;
            QString fileContent = fileInfo.fileContent;

            QList<QStandardItem *> newRow;
            newRow.append(new QStandardItem(fileName));
            newRow.append(new QStandardItem(fileSize));
            newRow.append(new QStandardItem(filePath));
            newRow.append(new QStandardItem(fileContent));
            model->appendRow(newRow);
        }
    }

private:
    QString dirPath;
    QList<MyFileInfo>traverse(const QString &dirPath)
    {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        QList<MyFileInfo> ret;
        for (QFileInfo &fileInfo : files)
        {
            if (fileInfo.isDir())
            {
                QList<MyFileInfo> xs = FileTraverseThread::traverse(fileInfo.absoluteFilePath());
                ret += xs;
            }
            else
            {
                if (fileInfo.isFile())
                {
                    MyFileInfo cfi;
                    cfi.fileName = fileInfo.fileName();
                    cfi.fileSize = fileInfo.size();
                    cfi.filePath = fileInfo.absoluteFilePath();
                    cfi.fileContent = FileTraverseThread::checkFileContent(cfi.filePath);
                    ret.append(cfi);

                    emit updateProgress(fileInfo.fileName());
                }
            }
        }
        return ret;
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
        file.close();
        return content.isEmpty() ? "Empty file!" : content;
    }
};

#endif // FILETRAVERSETHREAD_H
