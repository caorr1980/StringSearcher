#ifndef SEARCHERDIALOG_H
#define SEARCHERDIALOG_H

#include <QtGui>

namespace Ui {
class SearcherDialog;
}

class SearchThread;

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

    void on_BTN_Stop_clicked();

    void slot_show_result(const QString filePath, const int line, const QString context);

signals:
    void signal_Stop();

private:
    Ui::SearcherDialog *ui;

    void readSettings();
    void saveSettings();
    void updateComboBox(QComboBox *comboBox, QString &text);
    void searchDirectory(QDir &dir, QStringList &filterInList,
                         QStringList &filterOutList, QString &key);

    QDir searchDir;
    QFutureWatcher<void> cntWatcher;
    SearchThread *searchThread;
    bool stopSearch;
    int fileScanned;
    int fileMatched;
};

class SearchThread : public QThread
{
    Q_OBJECT

public:
    SearchThread(QString k):stopSearch(false), key(k){}

signals:
    void signal_show_result(const QString filePath, const int line, const QString context);

private slots:
    void slot_Stop(){stopSearch = true;}

private:
    void run();
    bool searchString(const QString &filePath, const QString &key);

    bool stopSearch;
    QString key;
};

#endif // SEARCHERDIALOG_H
