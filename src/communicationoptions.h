/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#ifndef COMMUNICATIONOPTIONS_H
#define COMMUNICATIONOPTIONS_H

#include <QDialog>

namespace Ui {
class CommunicationOptions;
}

class CommunicationOptions : public QDialog
{
    Q_OBJECT
    
public:
    explicit CommunicationOptions(QWidget *parent = 0);
    ~CommunicationOptions();
    void init();
    
private slots:
    void on_buttonBox_accepted();

    void on_cbServerModeConType_currentIndexChanged(int index);

    void on_cbClientModeConType_currentIndexChanged(int index);

private:
    Ui::CommunicationOptions *ui;

	void updateConType();
};

#endif // COMMUNICATIONOPTIONS_H
