#ifndef FILETRAVERSETHREAD_H
#define FILETRAVERSETHREAD_H

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

class FileTraverseWorker : public QObject
{
    Q_OBJECT

public:
    bool stopRequested;
    explicit FileTraverseWorker(const QString &dirPath, const QString &target, const QList<QString> &extensions)
        : dirPath(dirPath), target(target), extensions(extensions) {}

    void run() {
        stopRequested = false;
        const QList<MyFileInfo> cfi = FileTraverseWorker::traverse(dirPath, target, extensions);
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

signals:
    void updateProgress(QString file, bool ship);
    void updateFileInfo(QStandardItemModel *model);


private:
    QString dirPath;
    QString target;
    QList<QString> extensions;
    QList<MyFileInfo>traverse(const QString &dirPath, const QString &target, const QList<QString> &extensions)
    {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        QList<MyFileInfo> ret;
        for (QFileInfo &fileInfo : files)
        {
            if (!stopRequested)
            {
                if (fileInfo.isDir())
                {
                    QList<MyFileInfo> xs = FileTraverseWorker::traverse(fileInfo.absoluteFilePath(), target, extensions);
                    ret += xs;
                }
                else
                {
                    if (fileInfo.isFile())
                    {
                        QString fileName = fileInfo.fileName();
                        QStringList words = fileName.split(".");
                        if (words.length() >1)
                        {
                            QString ext = words.last();
                            ext = ext.trimmed();
                            if (extensions.contains(ext))
                            {
                                MyFileInfo cfi;
                                cfi.fileName = fileInfo.fileName();
                                cfi.fileSize = fileInfo.size();
                                cfi.filePath = fileInfo.absoluteFilePath();
                                cfi.fileContent = FileTraverseWorker::checkFileContent(cfi.filePath, target);
                                if (cfi.fileContent.length() > 0) {
                                    ret.append(cfi);
                                }

                                emit updateProgress(fileInfo.fileName(), false);
                            }
                            else
                               emit updateProgress(fileInfo.fileName(), true);
                        }
                        else
                            emit updateProgress(fileName, true);
                    }
                }
            }

        }
        return ret;
    }
    QString checkFileContent(const QString &filePath, const QString &target) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString msg = "Can not open file: ";
            msg += filePath;
            return msg;
        }

        QTextStream in(&file);
        // QString content = in.read(10);
        QString content = in.readAll();
        file.close();

        return content.contains(target) ? content : "";
    }
};

#endif // FILETRAVERSETHREAD_H
