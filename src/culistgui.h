/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#ifndef LISTRAQGUI_H
#define LISTRAQGUI_H

#define CULIST_VERSION "0.1.1"

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

/*
class AstmFieldInfo
{
public:
	QString _name;
	QString _defaultValue;
	bool _used;
	QString _constaints;
	AstmFieldInfo():_used(true){}
};

class AstmRecordInfo
{
public:
	QMap<QString, AstmFieldInfo> _fields;
	bool _used;
	AstmRecordInfo():_used(true){}
};

class AstmProfile
{
public:
	QString _name;
	QMap<QString, AstmRecordInfo> _records;
};
*/

class ProjectData
{
public:
	QString _fn;
	QString _currentProfile;
	//QMap<QString, AstmProfile> _profiles;
	unsigned short _serverModePort;
	QString _clientModeServer;
	QString _clientModeServerPort;	
	bool _proxyMode;
	ProjectData()
	{
		reset();
	}

	void reset()
	{
		_fn.clear();
		_currentProfile="ASTM_E1394_E97";
		
		_serverModePort = 0;
		_clientModeServer.clear();
		_clientModeServerPort="0";
		_proxyMode=false;

	}
};

class CulistGui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CulistGui(QWidget *parent = 0);
    ~CulistGui();

	bool loadTrace(QString tf);
	void clearMessages();
	void clearLog();

private:
	void createToolBars();
    
private slots:
    void on_actionExit_triggered();
    void on_actionLoad_Trace_triggered();
	void connectToTcpServer();

	void onConnnected();
	void onDataRead();
	void onError( QAbstractSocket::SocketError );
	void processConnectedState();
	void processDisconnectedState();
	void on_actionAbout_activated();
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
	void on_trvEditRecords_activated( const QModelIndex & index );
	void processCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous );
	void on_trvEditRecords_entered( const QModelIndex & index );
	void processRecordSelected( const QModelIndex & index );
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
	
    void on_actionSend_Data_triggered();	
	void on_actionRemove_Item_triggered();	
	
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

    void on_actionExport_triggered();

    //void on_lvProfileRecords_clicked(const QModelIndex &index);

    void on_bExportProfile_clicked();

    void on_bSaveProfile_clicked();

    void on_bCloneProfile_clicked();

    void on_bNewProfile_clicked();

    void on_bDeleteProfile_clicked();

    void on_cbCurrentProfile_currentIndexChanged(const QString &arg1);

private:
	void setCurrentProfile( QString );
	void saveProject();
	bool dataFromMessage( QStandardItem *mesg, QList<QByteArray> &outData );
	bool createSendData();
	void updateRecordView();
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
	ProjectData _projectData;
	QTimer *_sendDataTimer;

	QToolBar *_editToolBar;
	QToolBar *_connectToolBar;
	QToolBar *_runToolBar;
	QToolBar *_fileToolBar;
	QString _winTitleBase;
	QStandardItemModel _profileFields;
	//QStandardItemModel _profileRecords;
};

#endif // LISTRAQGUI_H
