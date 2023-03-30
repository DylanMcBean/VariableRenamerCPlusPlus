#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QKeyEvent>

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

void MainWindow::SetVarText()
{
    // Set variable count
    QString text = QString::number(m_current_variable_index + 1) + " / " + QString::number(m_variables.size());
    ui->label_var_count->setText(text);

    // Set Starter Variable
    if (!m_variables.isEmpty()) {
        // Get the first variable and its lines
        QString currentVar = m_variables.keys().at(m_current_variable_index);
        QList<QString> lines = m_variableLines.value(currentVar);

        // Set the text edits
        ui->label_var->setText("\"" + currentVar + "\" occurences");
        ui->textEdit_var_before->setText(currentVar);
        ui->textEdit_var_after->setText(m_variables.value(currentVar));
        ui->textEdit_var_lines->setText(lines.join("\n"));
    }
}


void MainWindow::on_actionLoad_triggered()
{
    QSet<QString> bannedWords = {
        "NULL", "true", "false", "nullptr", "__FILE__", "__LINE__",
        "int", "float", "double", "char", "bool", "void",
        "short", "long", "signed", "unsigned",
        "wchar_t", "char16_t", "char32_t",
        "size_t", "ptrdiff_t",
        "auto", "this",
        // control structures
        "if", "else", "switch", "case", "default",
        // loops
        "for", "while", "do",
        // functions
        "return", "throw"
    };

    QRegularExpression pattern("((?<!\\d|:|\\.|>)\\b[A-Za-z_]\\w*)\\b(?: ==?|,|;| ?\\))");

    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Select one or more files to open"), "", tr("All files (*.*);;Text files (*.txt)"));

    if (fileNames.isEmpty()) {
        return; // user cancelled the dialog
    }

    m_cpp_files = fileNames;

    QMap<QString, QList<QString>> variableLines;
    QMap<QString, QString> variables;

    foreach (const QString &fileName, fileNames) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file: ") + fileName);
            return;
        }
        QTextStream in(&file);
        int lineNumber = 0;
        while (!in.atEnd()) {
            QString line = in.readLine();
            ++lineNumber;
            QRegularExpressionMatchIterator matchIter = pattern.globalMatch(line);
            while (matchIter.hasNext()) {
                QRegularExpressionMatch match = matchIter.next();
                QString variable = match.captured(1);
                if (!bannedWords.contains(variable)) {
                    variables[variable] = variable;
                    if (variableLines.contains(variable)) {
                        variableLines[variable].append(line);
                    } else {
                        QList<QString> lines;
                        lines.append(line);
                        variableLines.insert(variable, lines);
                    }
                }
            }
        }
        file.close();
    }
    m_variableLines = variableLines;
    m_variables = variables;
    m_current_variable_index = 0;

    SetVarText();
}

void MainWindow::on_pushButton_next_clicked()
{
    if (!m_variables.isEmpty()) {
        int initial_index = m_current_variable_index;
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            // Shift key is pressed, keep incrementing the variable index
            do {
                m_current_variable_index = (m_current_variable_index + 1) % m_variables.size();
                SetVarText();
                on_textEdit_var_after_textChanged(); // call the function inside the loop
            } while (ui->textEdit_var_conflict_before->toPlainText() == "" && m_current_variable_index != initial_index);
        } else {
            // Shift key is not pressed, increment variable index once
            m_current_variable_index = (m_current_variable_index + 1) % m_variables.size();
            SetVarText();
        }
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    if (!m_variables.isEmpty()) {
        int initial_index = m_current_variable_index;
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            // Shift key is pressed, keep decrementing the variable index
            do {
                m_current_variable_index = (m_current_variable_index + m_variables.size() - 1) % m_variables.size();
                SetVarText();
                on_textEdit_var_after_textChanged(); // call the function inside the loop
            } while (ui->textEdit_var_conflict_before->toPlainText() == "" && m_current_variable_index != initial_index);
        } else {
            // Shift key is not pressed, decrement variable index once
            m_current_variable_index = (m_current_variable_index + m_variables.size() - 1) % m_variables.size();
            SetVarText();
        }
    }
}

void MainWindow::on_textEdit_var_after_textChanged()
{
    // check for conflicts
    QString before = ui->textEdit_var_before->toPlainText();
    QString after = ui->textEdit_var_after->toPlainText();
    m_variables[before] = after;
    bool conflict = false;
    for (auto it = m_variables.begin(); it != m_variables.end(); it ++)
    {
        if (after == it.value() && before != it.key())
        {
            ui->label_var_conflict->setText("\"" + it.key() + "\" occurences");
            ui->textEdit_var_conflict_before->setText(it.key());
            ui->textEdit_var_conflict_after->setText(it.value());
            ui->textEdit_var_conflict_lines->setText(m_variableLines.value(it.key()).join("\n"));
            conflict = true;
        }
    }
    if (!conflict)
    {
        ui->label_var_conflict->setText("{variable} occurences");
        ui->textEdit_var_conflict_before->clear();
        ui->textEdit_var_conflict_after->clear();
        ui->textEdit_var_conflict_lines->clear();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        // Call a function for the left arrow key
        on_pushButton_back_clicked();
    } else if (event->key() == Qt::Key_Right) {
        // Call a function for the right arrow key
        on_pushButton_next_clicked();
    } else {
        // Call the default implementation for other keys
        QMainWindow::keyPressEvent(event);
    }
}

bool MainWindow::IsConflict() {
    QSet<QString> uniqueValues;
    for (const auto& value : m_variables) {
        if (uniqueValues.contains(value))
            return true;
        uniqueValues.insert(value);
    }
    // no conflicts found
    return false;
}

void MainWindow::on_actionSave_triggered()
{
    if (!IsConflict()) {
        // Loop over the files
        for (const QString& fileName : m_cpp_files) {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << "Failed to open file" << fileName;
                continue;
            }

            // Read the file contents
            QTextStream in(&file);
            QString fileContents = in.readAll();
            file.close();

            // Loop over the variables
            for (const QString& variable : m_variables.keys()) {
                // Replace the variable with its value using regex
                QRegularExpression regex("\\b" + variable + "\\b");
                QString replacement = m_variables.value(variable);
                fileContents.replace(regex, replacement);
            }

            // Write the modified contents back to the file
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                qWarning() << "Failed to open file" << fileName << "for writing";
                continue;
            }

            QTextStream out(&file);
            out << fileContents;
            file.close();
            CleanUp();
        }
    } else if (!m_cpp_files.empty()) {
        ui->label_var_count->setText("Cannot Save, There are Conflicts");
    }
}

void MainWindow::CleanUp(){
    m_variables.clear();
    m_variableLines.clear();
    m_cpp_files.clear();
    ui->textEdit_var_lines->clear();
    ui->textEdit_var_conflict_lines->clear();
    ui->textEdit_var_conflict_after->clear();
    ui->textEdit_var_after->clear();
    ui->textEdit_var_conflict_before->clear();
    ui->textEdit_var_before->clear();
    ui->label_var_count->clear();
    ui->label_var->setText("{variable} occurences");
    ui->label_var_conflict->setText("{variable} occurences");
}

