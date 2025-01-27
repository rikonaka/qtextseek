#ifndef FILETRAVERSETHREAD_H
#define FILETRAVERSETHREAD_H

#include <QThread>
#include <QString>
#include <QDir>
#include <QFileInfoList>
#include <QStandardItemModel>

class FileTraverseThread : public QThread
{
    Q_OBJECT

public:
    explicit FileTraverseThread(const QString &dirPath, QObject *parent = nullptr)
        : QThread(parent), dirPath(dirPath) {}

signals:
    void progressUpdatedTotal(int value);
    void progressUpdated(int value);
    void finished();

protected:
    void run() override {
        QList<QFileInfo *> allFiles = FileTraverseThread::traverse(dirPath);
        emit progressUpdatedTotal(allFiles.length());

        QStandardItemModel *model;

        for (QFileInfo *fileInfo : allFiles) {
            QString fileName = fileInfo->fileName();
            QString fileSize = QString::number(fileInfo.size()) + " bytes";
            QString filePath = fileInfo->absoluteFilePath();
            QString fileContent = FileTraverseThread::checkFileContent(filePath);

            QList<QStandardItem *> newRow;
            newRow.append(new QStandardItem(fileName));
            newRow.append(new QStandardItem(fileSize));
            newRow.append(new QStandardItem(filePath));
            newRow.append(new QStandardItem(fileContent));
            model->appendRow(newRow);
        }

        emit

        emit finished();
    }

private:
    QString dirPath;
    QList<QFileInfo *>traverse(const QString &dirPath)
    {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        QList<QFileInfo *> ret;
        for (QFileInfo &fileInfo : files)
        {
            if (fileInfo.isDir())
            {
                QList<QFileInfo *> xs = FileTraverseThread::traverse(fileInfo.absoluteFilePath());
                ret += xs;
            }
            else
            {
                if (fileInfo.isFile())
                {
                    ret.append(&fileInfo);
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
        return content.isEmpty() ? "Empty file!" : content;
    }
};

#endif // FILETRAVERSETHREAD_H
