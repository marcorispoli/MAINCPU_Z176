#include "motortool.h"
#include "ui_motortool.h"

MotorTool::MotorTool(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MotorTool)
{
    ui->setupUi(this);
}

MotorTool::~MotorTool()
{
    delete ui;
}
