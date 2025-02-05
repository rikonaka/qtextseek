#ifndef FILETRAVERSETHREAD_H
#define FILETRAVERSETHREAD_H

#include <QString>
#include <QDir>
#include <QFileInfoList>
#include <QStandardItemModel>

class FileTraverseWorker : public QObject
{
    Q_OBJECT

public:
    bool stopRequested;
    explicit FileTraverseWorker(const QString &dirPath, const QString &target, const QList<QString> &extensions)
        : dirPath(dirPath), target(target), extensions(extensions) {}

    void run() {
        stopRequested = false;

        QStandardItemModel *model = new QStandardItemModel;
        model->setHorizontalHeaderLabels({tr("name"), tr("size"), tr("path"), tr("content")});
        emit updateFileInfo(model);
        dataModel = model;

        FileTraverseWorker::traverse(dirPath, target, extensions);
    }

signals:
    void updateProgress(QString file, bool ship);
    void updateFileInfo(QStandardItemModel *model);


private:
    QString dirPath;
    QString target;
    QList<QString> extensions;
    QStandardItemModel *dataModel;
    void traverse(const QString &dirPath, const QString &target, const QList<QString> &extensions)
    {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        for (QFileInfo &fileInfo : files)
        {
            if (!stopRequested)
            {
                if (fileInfo.isDir())
                    FileTraverseWorker::traverse(fileInfo.absoluteFilePath(), target, extensions);
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
                                QString fileName = fileInfo.fileName();
                                QString fileSize = QString::number(fileInfo.size()) + " bytes";
                                QString filePath = fileInfo.absoluteFilePath();
                                QString fileContent = FileTraverseWorker::checkFileContent(filePath, target);

                                QList<QStandardItem *> newRow;
                                newRow.append(new QStandardItem(fileName));
                                newRow.append(new QStandardItem(fileSize));
                                newRow.append(new QStandardItem(filePath));
                                newRow.append(new QStandardItem(fileContent));
                                dataModel->appendRow(newRow);

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
        // QString content = in.readAll();
        QString content;
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

        file.close();

        return content.contains(target) ? content : "";
    }
};

#endif // FILETRAVERSETHREAD_H
