/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#ifndef LISTRAQGUI_H
#define LISTRAQGUI_H

#include <QMainWindow>
#include <QModelIndex>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QTcpSocket>
#include <QTcpServer>
#include <QQueue>
#include "Astm.h"

namespace Ui {
class CulistGui;
}

class CulistGui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CulistGui(QWidget *parent = 0);
    ~CulistGui();

	bool loadTrace(QString tf);
	void clearMessages();
	void clearLog();

    
private slots:
    void on_actionExit_triggered();
    void on_actionLoad_Trace_triggered();
	void connectToTcpServer();

	void onConnnected();
	void onDataRead();
	void onError( QAbstractSocket::SocketError );
	void processConnectedState();
	void processDisconnectedState();
	void on_actionAbout_Qt_activated();

	void traceDataSent( const QByteArray & data, QString dir );
	void traceDataReceived( const QByteArray & data, QString dir  );

	void acceptServerConnection();
	void processServerConnectionData( );
	void processServerConnectionError(QAbstractSocket::SocketError );
	void processServerConnectionClosed();
	void send(const char *data);

    void on_actionListen_triggered();
    void on_actionStop_Listening_triggered();
    void on_actionDisconnect_triggered();
    void on_actionConnect_triggered();
    void on_trvEditRecords_customContextMenuRequested(const QPoint &pos);
	void on_trvEditRecords_clicked( const QModelIndex & index );
	void clearRecordEditView();
    void on_actionAdd_Session_triggered();
	void on_actionAdd_Message_triggered();
    void on_actionAdd_Patient_triggered();
	void on_actionAdd_Order_triggered();
	void on_actionAdd_Result_triggered();
	void on_actionAdd_Comment_triggered();
	void on_actionAdd_Header_triggered();
	void on_actionAdd_Terminator_triggered();
	void on_actionAdd_Manufacturer_triggered();
	void on_actionAdd_Scientific_triggered();
	void on_actionAdd_Request_triggered();
	

	void on_actionInsert_Session_triggered();
	void on_actionInsert_Message_triggered();
    void on_actionInsert_Patient_triggered();
	void on_actionInsert_Order_triggered();
	void on_actionInsert_Result_triggered();
	void on_actionInsert_Comment_triggered();
	void insertRecord( RecordType rt );
    void on_actionSend_Data_triggered();	
	void processDataToSend();

private slots:
    void on_actionClear_All_triggered();

    void on_actionFork_triggered();

    void on_bClearLog_clicked();

	void on_bSaveEditedRecord_clicked();

    void on_actionSave_Trace_triggered();

    void on_actionNew_Project_triggered();

    void on_actionSave_Project_triggered();

    void on_actionSave_Project_As_triggered();

    void on_actionLoad_Project_triggered();

    void on_actionSave_Trace_As_triggered();
    void saveTrace(QString fn);
	void processSendDataTimeout();

private:
	bool dataFromMessage( QStandardItem *mesg, QList<QByteArray> &outData );
	bool createSendData();
	QTcpSocket * getSendSocket( );
public:
	QStandardItem * addRecord( RecordType rt );
	bool setRecord( PAstm rec );
	QList<QByteArray> currentData();

private:
    Ui::CulistGui *ui;
	QStandardItemModel _editRecords;	
	QObjectList _recordEditViews;

	QTcpSocket *_tcpConnection;
    QTcpServer _tcpServer;
    QTcpSocket *_tcpServerConnection;
	bool _actAsProxyServer;
	bool _connected;
	QModelIndex _currentEditItem;
	typedef QList<QByteArray> TSendData;
	typedef QQueue<TSendData> TSendMessages;
	TSendMessages _messagesToSend;
	TSendData _dataToSend;
	int _dataToSendCurIdx;
	int _currentSendAttempt;
	bool _dataSending;
	QString _scriptFile;
	QString _lastTraceFile;
	QTimer *_sendDataTimer;
};

#endif // LISTRAQGUI_H
