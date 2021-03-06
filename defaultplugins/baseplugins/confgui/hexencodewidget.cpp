/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexencodewidget.h"
#include "ui_hexencodewidget.h"

HexEncodeWidget::HexEncodeWidget(HexEncode *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HexEncodeWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HexEncodeWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    switch(transform->getType()) {
        case HexEncode::NORMAL:
            ui->normalRadioButton->setChecked(true);
            break;
        case HexEncode::ESCAPED_MIXED:
            ui->escapedMixedRadioButton->setChecked(true);
            break;
        case HexEncode::ESCAPED:
            ui->escapedRadioButton->setChecked(true);
            break;
        case HexEncode::CSTYLE:
            ui->cstyleRadioButton->setChecked(true);
            break;
        case HexEncode::CSV:
            ui->csvRadioButton->setChecked(true);
            break;
        default:
            ui->normalRadioButton->setChecked(true);
    }

    ui->addPrefixCheckBox->setChecked(transform->getAddHexPrefix());

    connect(ui->normalRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->escapedRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->escapedMixedRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->cstyleRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->csvRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->addPrefixCheckBox, SIGNAL(toggled(bool)), SLOT(onAddPrefixChanged(bool)));
}

HexEncodeWidget::~HexEncodeWidget()
{
    delete ui;
}

void HexEncodeWidget::onTypeChange()
{
    if (ui->normalRadioButton->isChecked())
        transform->setType(HexEncode::NORMAL);
    else if (ui->escapedRadioButton->isChecked())
        transform->setType(HexEncode::ESCAPED);
    else if (ui->escapedMixedRadioButton->isChecked())
        transform->setType(HexEncode::ESCAPED_MIXED);
    else if (ui->cstyleRadioButton->isChecked())
        transform->setType(HexEncode::CSTYLE);
    else
        transform->setType(HexEncode::CSV);
}

void HexEncodeWidget::onAddPrefixChanged(bool val)
{
    transform->setAddHexPrefix(val);
}
