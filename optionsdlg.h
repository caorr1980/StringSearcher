#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <QDialog>

namespace Ui {
class optionsDlg;
}

class optionsDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit optionsDlg(QWidget *parent = 0);
    ~optionsDlg();

public slots:
    void slot_save_options();
    void slot_nolarger_changed(int state);
    
private slots:
    void on_BTN_TextProgram_clicked();

private:
    Ui::optionsDlg *ui;
};

#endif // OPTIONSDLG_H
