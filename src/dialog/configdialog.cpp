﻿#include "configdialog.h"
#include "ui_configdialog.h"
#include "settings.h"
#include "roomscene.h"
#include "mainwindow.h"
#include "engine.h"

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // tab 1
    QString bg_path = Config.value("BackgroundImage").toString();
    if (!bg_path.startsWith(":"))
        ui->bgPathLineEdit->setText(bg_path);

    ui->bgMusicPathLineEdit->setText(Config.value("BackgroundMusic", "audio/system/background.mp3").toString());

    ui->enableEffectCheckBox->setChecked(Config.EnableEffects);

    ui->enableLastWordCheckBox->setEnabled(Config.EnableEffects);
    ui->enableLastWordCheckBox->setChecked(Config.EnableLastWord);
    connect(ui->enableEffectCheckBox, SIGNAL(toggled(bool)), ui->enableLastWordCheckBox, SLOT(setEnabled(bool)));

    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);

    bool enabled_full = QFile::exists("skins/fulldefaultSkin.layout.json");
    ui->fullSkinCheckBox->setEnabled(enabled_full);
    ui->fullSkinCheckBox->setChecked(enabled_full && Config.value("UseFullSkin", true).toBool());
    ui->noIndicatorCheckBox->setChecked(Config.value("NoIndicator", false).toBool());
    ui->noEquipAnimCheckBox->setChecked(Config.value("NoEquipAnim", false).toBool());

    ui->bgmVolumeSlider->setValue(100 * Config.BGMVolume);
    ui->effectVolumeSlider->setValue(100 * Config.EffectVolume);

    // tab 2
    ui->neverNullifyMyTrickCheckBox->setChecked(Config.NeverNullifyMyTrick);
    ui->autoTargetCheckBox->setChecked(Config.EnableAutoTarget);
    ui->intellectualSelectionCheckBox->setChecked(Config.EnableIntellectualSelection);
    ui->doubleClickCheckBox->setChecked(Config.EnableDoubleClick);
    ui->superDragCheckBox->setChecked(Config.EnableSuperDrag);
    ui->bubbleChatBoxKeepSpinBox->setSuffix(tr(" millisecond"));
    ui->bubbleChatBoxKeepSpinBox->setValue(Config.BubbleChatBoxKeepTime);
    ui->backgroundChangeCheckBox->setChecked(Config.EnableAutoBackgroundChange);

    connect(ui->checkBoxRecorderAutoSave, SIGNAL(toggled(bool)), ui->checkBoxRecorderNetworkOnly, SLOT(setEnabled(bool)));
    ui->checkBoxRecorderAutoSave->setChecked(Config.value("recorder/autosave", true).toBool());
    ui->checkBoxRecorderNetworkOnly->setChecked(Config.value("recorder/networkonly", true).toBool());

    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));

    QFont font = Config.AppFont;
    showFont(ui->appFontLineEdit, font);

    font = Config.UIFont;
    showFont(ui->textEditFontLineEdit, font);

    QPalette palette;
    palette.setColor(QPalette::Text, Config.TextEditColor);
    QColor color = Config.TextEditColor;
    int aver = (color.red() + color.green() + color.blue()) / 3;
    palette.setColor(QPalette::Base, aver >= 208 ? Qt::black : Qt::white);
    ui->textEditFontLineEdit->setPalette(palette);
}

void ConfigDialog::showFont(QLineEdit *lineedit, const QFont &font)
{
    lineedit->setFont(font);
    lineedit->setText(QString("%1 %2").arg(font.family()).arg(font.pointSize()));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a background image"),
        "image/system/backdrop/",
        tr("Images (*.png *.bmp *.jpg)"));

    if (!filename.isEmpty()) {
        QString app_path = QApplication::applicationDirPath();
        if (filename.startsWith(app_path))
            filename = filename.right(filename.length() - app_path.length() - 1);
        ui->bgPathLineEdit->setText(filename);

        Config.BackgroundImage = filename;
        Config.setValue("BackgroundImage", filename);

        emit bg_changed();
    }
}

void ConfigDialog::on_resetBgButton_clicked()
{
    ui->bgPathLineEdit->clear();

    QString filename = "image/system/backdrop/default.jpg";
    Config.BackgroundImage = filename;
    Config.setValue("BackgroundImage", filename);

    emit bg_changed();
}

void ConfigDialog::saveConfig()
{
    float volume = ui->bgmVolumeSlider->value() / 100.0;
    Config.BGMVolume = volume;
    Config.setValue("BGMVolume", volume);
    volume = ui->effectVolumeSlider->value() / 100.0;
    Config.EffectVolume = volume;
    Config.setValue("EffectVolume", volume);

    bool enabled = ui->enableEffectCheckBox->isChecked();
    Config.EnableEffects = enabled;
    Config.setValue("EnableEffects", enabled);

    enabled = ui->enableLastWordCheckBox->isChecked();
    Config.EnableLastWord = enabled;
    Config.setValue("EnableLastWord", enabled);

    enabled = ui->enableBgMusicCheckBox->isChecked();
    Config.EnableBgMusic = enabled;
    Config.setValue("EnableBgMusic", enabled);

    Config.setValue("UseFullSkin", ui->fullSkinCheckBox->isChecked());
    Config.setValue("NoIndicator", ui->noIndicatorCheckBox->isChecked());
    Config.setValue("NoEquipAnim", ui->noEquipAnimCheckBox->isChecked());

    Config.NeverNullifyMyTrick = ui->neverNullifyMyTrickCheckBox->isChecked();
    Config.setValue("NeverNullifyMyTrick", Config.NeverNullifyMyTrick);

    Config.EnableAutoTarget = ui->autoTargetCheckBox->isChecked();
    Config.setValue("EnableAutoTarget", Config.EnableAutoTarget);

    Config.EnableIntellectualSelection = ui->intellectualSelectionCheckBox->isChecked();
    Config.setValue("EnableIntellectualSelection", Config.EnableIntellectualSelection);

    Config.EnableDoubleClick = ui->doubleClickCheckBox->isChecked();
    Config.setValue("EnableDoubleClick", Config.EnableDoubleClick);

    Config.EnableSuperDrag = ui->superDragCheckBox->isChecked();
    Config.setValue("EnableSuperDrag", Config.EnableSuperDrag);

    Config.BubbleChatBoxKeepTime = ui->bubbleChatBoxKeepSpinBox->value();
    Config.setValue("BubbleChatBoxKeepTime", Config.BubbleChatBoxKeepTime);

    Config.EnableAutoBackgroundChange = ui->backgroundChangeCheckBox->isChecked();
    Config.setValue("EnableAutoBackgroundChange", Config.EnableAutoBackgroundChange);

    enabled = ui->checkBoxRecorderAutoSave->isChecked();
    Config.setValue("recorder/autosave", enabled);
    enabled = ui->checkBoxRecorderNetworkOnly->isChecked();
    Config.setValue("recorder/networkonly", enabled);

    /*if (RoomSceneInstance)
        RoomSceneInstance->updateVolumeConfig();*/
    MainWindow *mw = static_cast<MainWindow*>(Sanguosha->parent());
    if (qobject_cast<RoomScene*>(mw->getScene()) == RoomSceneInstance && RoomSceneInstance != NULL)
        RoomSceneInstance->updateVolumeConfig();
}

void ConfigDialog::on_browseBgMusicButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a background music"),
        "audio/system",
        tr("Audio files (*.mp3)"));
    if (!filename.isEmpty()) {
        QString app_path = QApplication::applicationDirPath();
        if (filename.startsWith(app_path))
            filename = filename.right(filename.length() - app_path.length() - 1);
        ui->bgMusicPathLineEdit->setText(filename);
        Config.setValue("BackgroundMusic", filename);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked()
{
    QString default_music = "audio/system/background.mp3";
    Config.setValue("BackgroundMusic", default_music);
    ui->bgMusicPathLineEdit->setText(default_music);
}

void ConfigDialog::on_changeAppFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.AppFont, this);
    if (ok) {
        Config.AppFont = font;
        showFont(ui->appFontLineEdit, font);

        Config.setValue("AppFont", font);
        QApplication::setFont(font);
    }
}


void ConfigDialog::on_setTextEditFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.UIFont, this);
    if (ok) {
        Config.UIFont = font;
        showFont(ui->textEditFontLineEdit, font);

        Config.setValue("UIFont", font);
        QApplication::setFont(font, "QTextEdit");
    }
}

void ConfigDialog::on_setTextEditColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Config.TextEditColor, this);
    if (color.isValid()) {
        Config.TextEditColor = color;
        Config.setValue("TextEditColor", color);
        QPalette palette;
        palette.setColor(QPalette::Text, color);
        int aver = (color.red() + color.green() + color.blue()) / 3;
        palette.setColor(QPalette::Base, aver >= 208 ? Qt::black : Qt::white);
        ui->textEditFontLineEdit->setPalette(palette);
    }
}

