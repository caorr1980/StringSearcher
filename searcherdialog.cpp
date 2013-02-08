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
    headers << tr("File") << tr("Line") << tr("Context");
    ui->TBW_Result->setHorizontalHeaderLabels(headers);
    ui->TBW_Result->setColumnWidth(0, 300);
    ui->TBW_Result->setColumnWidth(1, 60);

    readSettings();
}

SearcherDialog::~SearcherDialog()
{
    saveSettings();
    delete ui;
}

void SearcherDialog::readSettings()
{

    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    QStringList textList;
    int i;

    settings.beginGroup("History");

    /* restore Keyword history */
    textList = settings.value("key").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Keyword->addItem(textList[i]);

    /* restore Replace word history */
    textList = settings.value("replace").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Replace->addItem(textList[i]);
    if (textList.size() > 0 && textList[0].isEmpty()) {
        ui->CMB_Replace->removeItem(0);
        ui->CMB_Replace->setEditText("");
    }

    /* restore Filter history */
    textList = settings.value("filter").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Filter->addItem(textList[i]);
    if (textList.size() > 0 && textList[0].isEmpty()) {
        ui->CMB_Filter->removeItem(0);
        ui->CMB_Filter->setEditText("");
    }

    /* restore searching dir history */
    textList = settings.value("dir").toStringList();
    if (textList.isEmpty())
        textList.append(QDir::currentPath());
    for (i = 0; i < textList.size(); i++)
        ui->CMB_DirPath->addItem(textList[i]);

    settings.endGroup();
}

void SearcherDialog::saveSettings()
{
    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    QStringList textList;
    int i;

    settings.beginGroup("History");

    /* save Keyword history */
    for (i = 0; i < ui->CMB_Keyword->count(); i++)
        textList.append(ui->CMB_Keyword->itemText(i));
    settings.setValue("key", textList);

    /* save Replace history */
    textList.clear();
    for (i = 0; i < ui->CMB_Replace->count(); i++)
        textList.append(ui->CMB_Replace->itemText(i));
    if (ui->CMB_Replace->currentText().isEmpty() && ui->CMB_Replace->count() > 0)
        textList.prepend("");
    settings.setValue("replace", textList);

    /* save Filter history */
    textList.clear();
    for (i = 0; i < ui->CMB_Filter->count(); i++)
        textList.append(ui->CMB_Filter->itemText(i));
    if (ui->CMB_Filter->currentText().isEmpty() && ui->CMB_Filter->count() > 0)
        textList.prepend("");
    settings.setValue("filter", textList);

    /* save Directory history */
    textList.clear();
    for (i = 0; i < ui->CMB_DirPath->count(); i++)
        textList.append(ui->CMB_DirPath->itemText(i));
    settings.setValue("dir", textList);

    settings.endGroup();
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
    QString path = ui->CMB_DirPath->currentText();
    updateComboBox(ui->CMB_DirPath, path);

    QStringList filterInList;
    QStringList filterOutList;
    QStringList filterList = filter.split(QRegExp("[ ,;]"), QString::SkipEmptyParts);
    filterList.removeDuplicates();
    for (int i = 0; i < filterList.count(); i++) {
        if (filterList[i].startsWith("-"))
            filterOutList.append(filterList[i].remove(0, 1));
        else
            filterInList.append(filterList[i]);

    }

    /* truncate previous results, and init result label */
    ui->TBW_Result->clearContents();
    ui->TBW_Result->setRowCount(0);
    ui->LBL_number->setText(tr("0 file(s) scanned, 0 file(s) matched."));

    fileScanned = 0;
    fileMatched = 0;
    searchDir = QDir(path);
    stopSearch = false;
    searchDirectory(searchDir, filterInList, filterOutList, key);
}

void SearcherDialog::searchDirectory(QDir &dir, QStringList &filterInList,
                                     QStringList &filterOutList, QString &key)
{
    if (stopSearch)
        return;

    QStringList fileList = dir.entryList(filterInList,
                       QDir::Files | QDir::NoSymLinks | QDir::AllDirs | QDir::NoDotAndDotDot);

    /* filter-out files that don't want to search */
    if (!filterOutList.isEmpty()) {
        QStringList fileOutList = dir.entryList(filterOutList,
                           QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);

        for (int i = 0; i < fileOutList.count(); i++) {
            fileList.removeAll(fileOutList[i]);
        }
    }

    for (int i = 0; i < fileList.size(); i++) {
        QString filePath = dir.absoluteFilePath(fileList[i]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            QDir subdir(filePath);
            searchDirectory(subdir, filterInList, filterOutList, key);
        } else {
            fileScanned++;
            if (searchString(filePath, key)) {
                fileMatched++;
            }
            ui->LBL_number->setText(tr("%1 file(s) scanned, %2 file(s) matched.")
                                    .arg(fileScanned).arg(fileMatched));
        }
    }
}

void SearcherDialog::updateComboBox(QComboBox *comboBox, QString &text)
{
    if (text.isEmpty())
        return;

    int index = comboBox->findText(text);

    if (index == -1) {
        comboBox->insertItem(0, text);
    } else {
        comboBox->removeItem(index);
        comboBox->insertItem(0, text);
        comboBox->setCurrentIndex(0);
    }
}

bool SearcherDialog::searchString(const QString &filePath, const QString &key, SearchMode md)
{
    bool found = false;
    QFile file(filePath);

    if (stopSearch || !file.open(QIODevice::ReadOnly)) {
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
                QString context = text.mid(index - 15);
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
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setFlags(item1->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 1, item1);

    QTableWidgetItem *item2 = new QTableWidgetItem(context);
    item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 2, item2);
}

void SearcherDialog::on_BTN_Stop_clicked()
{
    stopSearch = true;
}
