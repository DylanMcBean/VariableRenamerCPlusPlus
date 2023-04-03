#ifndef FILELINESMAP_H
#define FILELINESMAP_H

#include <QtCore>

class FileLinesMap : public QMap<QString, QList<QPair<int, QString>>>
{
public:
    FileLinesMap() {}
    ~FileLinesMap() {}

    void addLine(const QString &filename, int lineNumber, const QString &line)
    {
        qDebug() << line;
        if (!contains(filename)) {
            insert(filename, QList<QPair<int, QString>>());
        }
        value(filename).append(QPair<int, QString>(lineNumber, line));
    }

    QList<QPair<int, QString>> linesForFile(const QString &filename) const
    {
        return value(filename);
    }
};

#endif // FILELINESMAP_H
