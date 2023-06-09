#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // Set the focus policy and ensure the main window has focus
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    // Install event filter for the main window
    installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setTextWithHighlightedVar(QTextEdit *textEdit, const QString &text, const QString &currentVar) {
    QTextDocument *document = textEdit->document();
    document->setPlainText(text);

    QTextCursor cursor(document);
    QTextCharFormat normalFormat;
    QTextCharFormat highlightedFormat;
    highlightedFormat.setForeground(QColor("red"));
    highlightedFormat.setFontWeight(QFont::Bold);

    QTextDocument::FindFlags findFlags = QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords; // Add this line to set the find flags

    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = document->find(currentVar, cursor, findFlags);
        if (!cursor.isNull()) {
            cursor.mergeCharFormat(highlightedFormat);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, currentVar.length());
        }
    }

    textEdit->setTextCursor(cursor);
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

        // Use the custom function to set the text and highlight currentVar
        setTextWithHighlightedVar(ui->textEdit_var_lines, lines.join("\n"), currentVar);

        ui->tabWidget_2->clear();
        foreach (const QString& filename, m_cpp_files) {
            QWidget* dummy_tab = new QWidget();
            ui->tabWidget_2->addTab(dummy_tab, filename);
            QVBoxLayout* layout = new QVBoxLayout(dummy_tab);
            QTextEdit* textEdit = new QTextEdit(dummy_tab);
            layout->addWidget(textEdit);
            dummy_tab->setLayout(layout);

            textEdit->setLineWrapMode(QTextEdit::NoWrap);


            QList<QPair<int, QString>> uniqueLines = {};
            QList<QPair<int, QString>> lines = m_fileLinesMapping.value(filename);
            QString regexString = QString("\\b%1\\b").arg(currentVar); // create the regex string for exact word matching
            QRegularExpression regex(regexString); // create the regex object
            for (QList<QPair<int, QString>>::const_iterator line = lines.constBegin(); line != lines.constEnd(); ++line) {
                if (regex.match(line->second).hasMatch()) { // use the regex to match the variable name
                    for(int i = -5; i <= 5; i ++) {
                        int index = line->first + i;
                        if (index >= 0 && index < lines.size() && !uniqueLines.contains(lines.at(index))) {
                            uniqueLines.append(lines.at(index));
                        }
                    }
                }
            }

            QString filelines = "";
            int previousLine = -1;
            for (QList<QPair<int, QString>>::const_iterator line = uniqueLines.constBegin(); line != uniqueLines.constEnd(); ++line) {
                QString lineNumberStr = QString::number(line->first);
                QString paddedLineNumber = lineNumberStr.leftJustified(12, ' ');
                // add ellipses at the beginning of the file if the current line is more than 5 lines away from the first line
                if (previousLine == -1 && line->first > 5) {
                    filelines += "...\n\n";
                }
                // check if the difference between the current line number and the previous line number is greater than 5
                if (previousLine != -1 && line->first - previousLine > 5) {
                    filelines += "\n...\n\n"; // add ellipses to simulate where other code would be
                }
                filelines += QString("%1 %2\n").arg(paddedLineNumber).arg(line->second);
                previousLine = line->first;
            }
            // add ellipses at the end of the file if the last line is more than 5 lines away from the end of the file
            if (previousLine != -1 && uniqueLines.last().first < lines.last().first - 5) {
                filelines += "\n...\n";
            }

            setTextWithHighlightedVar(textEdit, filelines, currentVar);
        }
    }
}


void MainWindow::on_actionLoad_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Select one or more files to open"), "", tr("All files (*.*);;Text files (*.txt)"));

    if (fileNames.isEmpty()) {
        return; // user cancelled the dialog
    }

    m_cpp_files = fileNames;

    QSet<QString> uniqueVariables;
    QString preProcessorLines = R"(^#.*$)";
    QString strings = R"((?:L?R?"(?:[^"\\]|\\.)*"|L?'(?:[^'\\]|\\.)'))";
    QString multiLineComments = R"(/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/)";
    QString singleLineComments = R"(\s*//.*$)";
    QString functions = R"(\b[a-zA-Z_]\w*\b\()";
    QString keywords = R"(\b(?!alignas|alignof|and|and_eq|asm|atomic_cancel|atomic_commit|atomic_noexcept|auto|bitand|bitor|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|compl|concept|const|consteval|constexpr|constinit|const_cast|continue|co_await|co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|else|enum|explicit|export|extern|false|float|for|friend|goto|if|inline|int|long|mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|private|protected|public|reflexpr|register|reinterpret_cast|requires|return|short|signed|sizeof|static|static_assert|static_cast|struct|switch|synchronized|template|this|thread_local|throw|true|try|typedef|typeid|typename|union|unsigned|using|virtual|void|volatile|wchar_t|while|xor|xor_eq)\b[a-zA-Z_]\w*\b)";


    for (const QString &fileName : fileNames) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();

            content.replace(QRegularExpression(preProcessorLines, QRegularExpression::MultilineOption), " ");
            content.replace(QRegularExpression(strings), " ");
            content.replace(QRegularExpression(multiLineComments), " ");
            content.replace(QRegularExpression(singleLineComments, QRegularExpression::MultilineOption), " ");
            content.replace(QRegularExpression(functions), " ");
            QRegularExpression reKeywords(keywords);
            QRegularExpressionMatchIterator it = reKeywords.globalMatch(content);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                uniqueVariables.insert(match.captured(0));
            }
        }
    }

    QMap<QString, QList<QString>> variableLines;
    QMap<QString, QString> variables;
    QMap<QString, QList<QPair<int, QString>>> fileLinesMapping;

    for (const QString &fileName : fileNames) {
        int lineNumber = 0;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            QStringList lines = content.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
            file.close();

            if (!fileLinesMapping.contains(fileName))
            {
                fileLinesMapping.insert(fileName, QList<QPair<int, QString>>());
            }

            QList<QPair<int, QString>>& fileLines = fileLinesMapping[fileName];
            for (const QString &line : lines) {
                fileLines.append(QPair<int, QString>(lineNumber++, line));
            }

            for (const QString &variable : uniqueVariables) {
                QRegularExpression reVariable("\\b" + QRegularExpression::escape(variable) + "\\b");
                for (const QString &line : lines) {
                    if (reVariable.match(line).hasMatch()) {
                        if (!variables.contains(variable)) {
                            variables.insert(variable, variable);
                        }
                        if (!variableLines.contains(variable)) {
                            variableLines.insert(variable, QList<QString>());
                        }
                        variableLines[variable].append(line);
                    }
                }
            }
        }
    }

    m_variableLines = variableLines;
    m_variables = variables;
    m_current_variable_index = 0;
    m_fileLinesMapping = fileLinesMapping;

    SetVarText();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Check if the pressed key is either the left or right arrow key
        if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right)
        {
            QPushButton *targetButton = nullptr;

            // Determine which button to press
            if (keyEvent->key() == Qt::Key_Left)
            {
                targetButton = ui->pushButton_back;
            }
            else if (keyEvent->key() == Qt::Key_Right)
            {
                targetButton = ui->pushButton_next;
            }

            if (targetButton)
            {
                // Simulate button press
                targetButton->animateClick();
            }

            // The event is handled, no further propagation is needed
            return true;
        }
    }

    // If the event is not handled, pass it to the base class event filter
    return QMainWindow::eventFilter(watched, event);
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

            // Use the custom function to set the text and highlight currentVal
            setTextWithHighlightedVar(ui->textEdit_var_conflict_lines, m_variableLines.value(it.key()).join("\n"), it.key());

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
        }
        CleanUp();
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
