#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    ~MainWindow();

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
    int m_current_varible_index = 0;
    QStringList m_cpp_files;
};
#endif // MAINWINDOW_H
