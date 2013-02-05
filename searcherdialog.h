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
    
private slots:
    void on_BTN_Browse_clicked();

    void on_BTN_Search_clicked();

private:
    Ui::SearcherDialog *ui;

    void updateComboBox(QComboBox *comboBox, QString &text);
    void showFiles(const QStringList &files);
};

#endif // SEARCHERDIALOG_H
