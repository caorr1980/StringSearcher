#include <QDebug>
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

void SearcherDialog::on_BTN_Browse_clicked()
{
    QFileDialog::Options options =
                QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    QString dirPath = QFileDialog::getExistingDirectory(this,
                                tr("Please Select Path:"),
                                ui->CMB_DirPath->currentText(),
                                options);
    if (!dirPath.isEmpty()) {
        updateComboBox(ui->CMB_DirPath, dirPath);
        ui->CMB_DirPath->setCurrentIndex(ui->CMB_DirPath->findText(dirPath));
    }
}

void SearcherDialog::on_BTN_Search_clicked()
{
    /* truncate previous results */
    ui->TBW_Result->setRowCount(0);

    QString path = ui->CMB_DirPath->currentText();
    qDebug() << path;
}

void SearcherDialog::updateComboBox(QComboBox *comboBox, QString &text)
{
    if (comboBox->findText(text) == -1)
        comboBox->insertItem(0, text);
}
