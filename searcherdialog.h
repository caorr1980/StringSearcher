#ifndef SEARCHERDIALOG_H
#define SEARCHERDIALOG_H

#include <QDialog>

namespace Ui {
class SearcherDialog;
}

class SearcherDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SearcherDialog(QWidget *parent = 0);
    ~SearcherDialog();
    
private:
    Ui::SearcherDialog *ui;
};

#endif // SEARCHERDIALOG_H
