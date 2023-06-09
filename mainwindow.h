#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "filelinesmap.h"

#include <QMainWindow>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void SetVarText();
    void CheckForConflicts();
    void CleanUp();
    bool IsConflict();
    void setTextWithHighlightedVar(QTextEdit *textEdit, const QString &text, const QString &currentVar);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void on_actionLoad_triggered();
    void on_pushButton_next_clicked();
    void on_pushButton_back_clicked();
    void on_textEdit_var_after_textChanged();
    void keyPressEvent(QKeyEvent *event);
    void on_actionSave_triggered();

private:
    QMap<QString, QString> m_variables;
    QMap<QString, QList<QString>> m_variableLines;
    Ui::MainWindow *ui;
    int m_current_variable_index = 0;
    QStringList m_cpp_files;
    QMap<QString, QList<QPair<int, QString>>> m_fileLinesMapping;
};
#endif // MAINWINDOW_H
