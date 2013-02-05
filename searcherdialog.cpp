#include <QDebug>
#include "searcherdialog.h"
#include "ui_searcherdialog.h"

SearcherDialog::SearcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearcherDialog)
{
    ui->setupUi(this);

    /* rename TBW_Result's horizontal header */
    QStringList headers;
    headers << tr("File");
    ui->TBW_Result->setHorizontalHeaderLabels(headers);
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
    QString key = ui->CMB_Keyword->currentText();
    if (key.isEmpty()) {
        QMessageBox::warning(this, tr("No KeyWord"),
                         tr("Please specify the Keyword to search!"));
        qDebug() << "No KeyWord";
        return;
    }

    /* truncate previous results */
    ui->TBW_Result->setRowCount(0);

    QString fileFilter = ui->CMB_Filter->currentText();
    if (fileFilter.isEmpty())
        fileFilter = "*";

    QString path = ui->CMB_DirPath->currentText();
    QDir curDir = QDir(path);
    QStringList files = curDir.entryList(QStringList(fileFilter),
                                 QDir::Files | QDir::NoSymLinks);
//    if (!text.isEmpty())
//        files = findFiles(files, text);
    showFiles(files);
}

void SearcherDialog::updateComboBox(QComboBox *comboBox, QString &text)
{
    if (comboBox->findText(text) == -1)
        comboBox->insertItem(0, text);
}

void SearcherDialog::showFiles(const QStringList &files)
{
    int row;

    for (int i = 0; i < files.size(); i++) {
        qDebug() << files[i];
        QTableWidgetItem *fileItem = new QTableWidgetItem(files[i]);
        fileItem->setFlags(fileItem->flags() ^ Qt::ItemIsEditable);

        row = ui->TBW_Result->rowCount();
        ui->TBW_Result->insertRow(row);
        ui->TBW_Result->setItem(row, 0, fileItem);
    }

    ui->LBL_number->setText(tr("%1 file(s) found").arg(files.size()));
    ui->LBL_number->setWordWrap(true);
}
