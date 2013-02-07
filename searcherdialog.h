#ifndef SEARCHERDIALOG_H
#define SEARCHERDIALOG_H

#include <QtGui>

namespace Ui {
class SearcherDialog;
}

class SearcherDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SearcherDialog(QWidget *parent = 0);
    ~SearcherDialog();

    enum SearchMode {
        QTMode       = 0x0001,
        BMMode       = 0x0002,
    };
    
private slots:
    void on_BTN_Browse_clicked();

    void on_BTN_Search_clicked();

private:
    Ui::SearcherDialog *ui;

    void readSettings();
    void saveSettings();
    void updateComboBox(QComboBox *comboBox, QString &text);
    void searchDirectory(QDir &dir, QStringList &filterInList, QStringList &filterOutList, QString &key);
    bool searchString(const QString &filePath, const QString &key, SearchMode md = QTMode);
    void showResult(const QString &filePath, const int &line, const QString &context);

    QDir searchDir;
    int fileMatched;
};

#endif // SEARCHERDIALOG_H
