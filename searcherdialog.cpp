#include <QDebug>
#include "searcherdialog.h"
#include "ui_searcherdialog.h"

SearcherDialog::SearcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearcherDialog)
{
    ui->setupUi(this);

    /* set default directory to search */
    ui->CMB_DirPath->setEditText(QDir::currentPath());

    /* rename TBW_Result's horizontal header */
    QStringList headers;
    headers << tr("File") << tr("Line") << tr("Context");
    ui->TBW_Result->setHorizontalHeaderLabels(headers);
    ui->TBW_Result->setColumnWidth(0, 300);
    ui->TBW_Result->setColumnWidth(1, 60);
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
        ui->CMB_DirPath->setEditText(dirPath);
    }
}

void SearcherDialog::on_BTN_Search_clicked()
{
    QString key = ui->CMB_Keyword->currentText();
    if (key.isEmpty()) {
        QMessageBox::warning(this, tr("No KeyWord"),
                         tr("Please specify the Keyword to search!"));
        return;
    }
    /* save history for each ComboBox */
    updateComboBox(ui->CMB_Keyword, key);
    QString replace = ui->CMB_Replace->currentText();
    updateComboBox(ui->CMB_Replace, replace);
    QString filter = ui->CMB_Filter->currentText();
    updateComboBox(ui->CMB_Filter, filter);

    if (filter.isEmpty())
        filter = "*";

    /* truncate previous results */
    ui->TBW_Result->clearContents();
    ui->TBW_Result->setRowCount(0);

    fileFoundNum = 0;

    QString path = ui->CMB_DirPath->currentText();
    updateComboBox(ui->CMB_DirPath, path);

    searchDir = QDir(path);
    searchDirectory(searchDir, filter, key);

    ui->LBL_number->setText(tr("%1 file(s) found").arg(fileFoundNum));
    ui->LBL_number->setWordWrap(true);
}

void SearcherDialog::searchDirectory(QDir &dir, QString &filter, QString &key)
{
    QStringList fileList = dir.entryList(QStringList(filter),
                       QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i < fileList.size(); i++) {
        QString filePath = dir.absoluteFilePath(fileList[i]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            QDir subdir(filePath);
            searchDirectory(subdir, filter, key);
        } else {
            if (searchString(filePath, key))
                fileFoundNum++;
        }
    }
}

void SearcherDialog::updateComboBox(QComboBox *comboBox, QString &text)
{
    if (text.isEmpty())
        return;

    if (comboBox->findText(text) == -1)
        comboBox->insertItem(0, text);
}

bool SearcherDialog::searchString(const QString &filePath, const QString &key, SearchMode md)
{
    bool found = false;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream in(&file);
    QString text;
    int line = 0;

    while(!in.atEnd()) {
        line++;
        if (md == QTMode) {
            text = in.readLine();
            if (text.contains(key)) {
                int index = text.indexOf(key);
                QString context = text.right(index - 20);
                showResult(filePath, line, context);
                found = true;
                break;
            }
        }
    }

    return found;
}

void SearcherDialog::showResult(const QString &filePath, const int &line, const QString &context)
{
    int row = ui->TBW_Result->rowCount();
    ui->TBW_Result->insertRow(row);

    QTableWidgetItem *item0 = new QTableWidgetItem(searchDir.relativeFilePath(filePath));
    item0->setFlags(item0->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(line));
    item1->setTextAlignment(Qt::AlignHCenter);
    item1->setFlags(item1->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 1, item1);

    QTableWidgetItem *item2 = new QTableWidgetItem(context);
    item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 2, item2);
}
