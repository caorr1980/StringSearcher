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
    void slot_update_label(const int fileScanned, const int fileMatched, const int fileFailed);
    void slot_thread_stopped();

private:
    Ui::SearcherDialog *ui;

    void readSettings();
    void saveSettings();
    void updateComboBox(QComboBox *comboBox, QString &text);
    void scanDirectory(QDir &dir, QStringList &filterInList,
                         QStringList &filterOutList, QString &key);
    void collectFiles(QDir &dir, QStringList &filterInList,
                         QStringList &filterOutList, QString &key);

    QTime ElapsedTime;
    QDir searchDir;
    QFutureWatcher<void> cntWatcher;
    SearchThread *searchThread;
    bool stopScanDir;
    bool isThreadWorking;
};

class SearchThread : public QThread
{
    Q_OBJECT

public:
    explicit SearchThread(QString k);
    ~SearchThread();

signals:
    void signal_show_result(const QString filePath, const int line, const QString context);
    void signal_update_label(const int fileScanned, const int fileMatched, const int fileFailed);

private:
    void run();
    void stop();
    bool searchString(const QString &filePath, const QString &key);

    QString key;
    bool stopSearchStr;
    int fileScanned;
    int fileMatched;
    int fileFailed;
};

#endif // SEARCHERDIALOG_H
