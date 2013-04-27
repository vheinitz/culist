/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#include "communicationoptions.h"
#include "ui_communicationoptions.h"

CommunicationOptions::CommunicationOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommunicationOptions)
{
    ui->setupUi(this);
}

CommunicationOptions::~CommunicationOptions()
{
    delete ui;
}

void CommunicationOptions::init()
{
	/*ui->cbServerModeConType->setCurrentIndex( Settings::instance().get("Comm.Server.Mode").toInt() );
	ui->cbClientModeConType->setCurrentIndex( Settings::instance().get("Comm.Client.Mode").toInt() );

	ui->eClientModeSrvAddr->setText( Settings::instance().get("Comm.Client.Srv.Addr").toString() );
	ui->eClientModeSrvPort->setText( Settings::instance().get("Comm.Client.Srv.Port").toString() );
	ui->eClientSerialPort->setText( Settings::instance().get("Comm.Client.Ser.Port").toString() );
	QString sp = Settings::instance().get("Comm.Server.Srv.Port").toString() ;
	ui->eServerModeSrvPort->setText( sp );
	ui->chbActAsProxy->setChecked( Settings::instance().get("Comm.Proxy.Mode").toInt() );
*/
	updateConType();
}

void CommunicationOptions::on_buttonBox_accepted()
{
	/*
	Settings::instance().set("Comm.Server.Mode", ui->cbServerModeConType->currentIndex() );
	Settings::instance().set("Comm.Client.Mode", ui->cbClientModeConType->currentIndex() );

	Settings::instance().set("Comm.Client.Srv.Addr", ui->eClientModeSrvAddr->text() );
	Settings::instance().set("Comm.Client.Srv.Port", ui->eClientModeSrvPort->text() );
	Settings::instance().set("Comm.Client.Ser.Port", ui->eClientSerialPort->text() );

	Settings::instance().set("Comm.Server.Srv.Port", ui->eServerModeSrvPort->text() );
	Settings::instance().set("Comm.Proxy.Mode", (int)ui->chbActAsProxy->isChecked() );
*/
}

void CommunicationOptions::updateConType()
{
	bool active = ui->cbServerModeConType->currentIndex() == 0; //TCP
	ui->lServerModeSrvPort->setEnabled( active );
	ui->eServerModeSrvPort->setEnabled( active );
	ui->bServerModeTestSrv->setEnabled( active );
	ui->bServerModeTestPort->setEnabled( !active );
	ui->lServerModeSerPort->setEnabled( !active );
	ui->eServerSerialPort->setEnabled( !active );

	active = ui->cbClientModeConType->currentIndex() == 0; //TCP
	ui->lClientModeSrvPort->setEnabled( active );
	ui->eClientModeSrvPort->setEnabled( active );
	ui->bClientModeTestSrv->setEnabled( active );
	ui->lClientModeSerPort->setEnabled( !active );
	ui->eClientSerialPort->setEnabled( !active );
	ui->bClientModeTestPort->setEnabled( !active );
}
void CommunicationOptions::on_cbServerModeConType_currentIndexChanged(int index)
{
    updateConType();
}

void CommunicationOptions::on_cbClientModeConType_currentIndexChanged(int index)
{
    updateConType();
}
