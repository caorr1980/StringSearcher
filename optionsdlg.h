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
    
private:
    Ui::optionsDlg *ui;
};

#endif // OPTIONSDLG_H
