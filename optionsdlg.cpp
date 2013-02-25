#include <QDebug>
#include <QtGui>
#include "optionsdlg.h"
#include "ui_optionsdlg.h"

optionsDlg::optionsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsDlg)
{
    ui->setupUi(this);

    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    bool state;
    QString Str;
    int idx;

    settings.beginGroup("Options");

    /* Options */
    state = settings.value("CaseSensitive").toBool();
    ui->CKB_Case->setChecked(state);
    state = settings.value("WholeWordsOnly").toBool();
    ui->CKB_WholeWords->setChecked(state);
    state = settings.value("IncludeSubdir", true).toBool();
    ui->CKB_Subdir->setChecked(state);
    state = settings.value("EnglisthTextOnly").toBool();
    ui->CKB_TextOnly->setChecked(state);
    state = settings.value("FirstResultOnly").toBool();
    ui->CKB_OnlyFirst->setChecked(state);
    state = settings.value("NoLarger").toBool();
    ui->CKB_NoLarger->setChecked(state);
    Str = settings.value("NoLargerSize").toString();
    ui->LNE_LargerSize->setText(Str);
    if (ui->CKB_NoLarger->isChecked())
        ui->LNE_LargerSize->setEnabled(true);
    state = settings.value("ReplaceMode").toBool();
    ui->CKB_ReplaceMode->setChecked(state);

    /* Settings */
    idx = settings.value("ThreadNum", 1).toInt();
    ui->CMB_ThreadNum->setCurrentIndex(idx);
    Str = settings.value("TextProgram", "C:/Windows/system32/notepad.exe").toString();
    ui->LNE_TextProgram->setText(Str);

    settings.endGroup();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_save_options()));
    connect(ui->CKB_NoLarger, SIGNAL(stateChanged(int)), this, SLOT(slot_nolarger_changed(int)));
}

optionsDlg::~optionsDlg()
{
    delete ui;
}

void optionsDlg::slot_save_options()
{
    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    bool state;
    QString Str;
    int idx;

    settings.beginGroup("Options");

    /* Options */
    state = ui->CKB_Case->isChecked();
    settings.setValue("CaseSensitive", state);
    state = ui->CKB_WholeWords->isChecked();
    settings.setValue("WholeWordsOnly", state);
    state = ui->CKB_Subdir->isChecked();
    settings.setValue("IncludeSubdir", state);
    state = ui->CKB_TextOnly->isChecked();
    settings.setValue("EnglisthTextOnly", state);
    state = ui->CKB_OnlyFirst->isChecked();
    settings.setValue("FirstResultOnly", state);
    state = ui->CKB_NoLarger->isChecked();
    settings.setValue("NoLarger", state);
    Str = ui->LNE_LargerSize->text();
    settings.setValue("NoLargerSize", Str);
    state = ui->CKB_ReplaceMode->isChecked();
    settings.setValue("ReplaceMode", state);

    /* Settings */
    idx = ui->CMB_ThreadNum->currentIndex();
    settings.setValue("ThreadNum", idx);
    Str = ui->LNE_TextProgram->text();
    settings.setValue("TextProgram", Str);

    settings.endGroup();
}

void optionsDlg::slot_nolarger_changed(int state)
{
    if (state == Qt::Unchecked)
        ui->LNE_LargerSize->setEnabled(false);
    else
        ui->LNE_LargerSize->setEnabled(true);
}

void optionsDlg::on_BTN_TextProgram_clicked()
{
    QFileDialog::Options options = QFileDialog::ReadOnly;
    QString selectedFilter = tr("Executables (*.exe)");

    qDebug() << ui->LNE_TextProgram->text();

    QString ProgramPath = QFileDialog::getOpenFileName(this,
                                tr("Please Select the Program:"),
                                ui->LNE_TextProgram->text(),
                                tr("Executables (*.exe);;All (*)"),
                                &selectedFilter,
                                options);
    if (!ProgramPath.isEmpty())
        ui->LNE_TextProgram->setText(ProgramPath);
}
