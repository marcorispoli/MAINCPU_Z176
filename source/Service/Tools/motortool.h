#ifndef MOTORTOOL_H
#define MOTORTOOL_H

#include <QWidget>

namespace Ui {
class MotorTool;
}

class MotorTool : public QWidget
{
    Q_OBJECT
    
public:
    explicit MotorTool(QWidget *parent = 0);
    ~MotorTool();
    
private:
    Ui::MotorTool *ui;
};

#endif // MOTORTOOL_H
