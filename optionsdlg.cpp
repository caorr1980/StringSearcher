#include "optionsdlg.h"
#include "ui_optionsdlg.h"

optionsDlg::optionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsDlg)
{
    ui->setupUi(this);
}

optionsDlg::~optionsDlg()
{
    delete ui;
}
