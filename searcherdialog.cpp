#include "searcherdialog.h"
#include "ui_searcherdialog.h"

SearcherDialog::SearcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearcherDialog)
{
    ui->setupUi(this);
}

SearcherDialog::~SearcherDialog()
{
    delete ui;
}
