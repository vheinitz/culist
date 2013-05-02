/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#include "culistgui.h"
#include "ui_culistgui.h"

#include <QFileDialog>
#include <QTextStream>
#include <QTimer>
#include <QProcess>
#include <QLabel>
#include <QLineEdit>
#include "persistence.h"


QMap<RecordType,QString> _recordNames;

CulistGui::CulistGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CulistGui),
	_tcpConnection(0),
	_tcpServerConnection(0),
	_actAsProxyServer(false),
	_dataToSendCurIdx(0),
	_currentSendAttempt(0),
	_dataSending(false)
{
    ui->setupUi(this);
	PERSISTENCE_INIT("heinitz-it","CULIST");
	PERSISTENT("WindowPosition", this, "geometry")
	PERSISTENT("ClientModeServerHost", ui->eClientModeSrvAddr, "text");
	PERSISTENT("ClientModeServerPort", ui->eClientModeSrvPort, "text");
	PERSISTENT("ServerModeServerHost", ui->eServerModeSrvPort, "text");


	_recordNames[EPatient] = tr("Patient");
	_recordNames[ERequest] = tr("Request");
	_recordNames[EScientific] = tr("Scientific");
	_recordNames[EOrder] = tr("Order");
	_recordNames[EResult] = tr("Result");
	_recordNames[EComment] = tr("Comment");
	_recordNames[EHeader] = tr("Header");
	_recordNames[ETerminator] = tr("Terminator");
	_recordNames[EManufacturer] = tr("Manufacturer");
	on_actionClear_All_triggered();
	
	ui->actionStop_Listening->setEnabled(false);
	ui->actionDisconnect->setEnabled(false);

	_sendDataTimer = new QTimer(this);
	_sendDataTimer->setSingleShot(true);
	_sendDataTimer->setInterval(3000);//TODO configurable
	connect( _sendDataTimer, SIGNAL(timeout()), this, SLOT(processSendDataTimeout()) );

	_projectData._profile = "astm_E1394E97";
	
	ASTMFactory::instance().init();

	
}

CulistGui::~CulistGui()
{
    delete ui;
}



void CulistGui::clearMessages()
{
	_editRecords.clear();
}

void CulistGui::clearLog()
{
	ui->tRecordContent->clear();
}


void CulistGui::on_actionExit_triggered()
{
    close();
}

void CulistGui::connectToTcpServer()
{
    if (_tcpConnection ) delete _tcpConnection;
    _tcpConnection = new QTcpSocket(this);
	

	_tcpConnection ->connectToHost( ui->eClientModeSrvAddr->text(), ui->eClientModeSrvPort->text().toUShort() );
    connect(_tcpConnection,SIGNAL(connected()), this, SLOT(onConnnected()));
    connect(_tcpConnection,SIGNAL(readyRead()), this, SLOT(onDataRead()));
    connect(_tcpConnection,SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void CulistGui::onConnnected()
{
    processConnectedState();
}

void CulistGui::acceptServerConnection()
{
    if(_tcpServerConnection == 0)
    {
        _tcpServerConnection = _tcpServer.nextPendingConnection();
        connect(_tcpServerConnection, SIGNAL(readyRead()),
                this, SLOT(processServerConnectionData()));
        connect(_tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(processServerConnectionError(QAbstractSocket::SocketError)));

		connect(_tcpServerConnection, SIGNAL(disconnected()),
                this, SLOT(processServerConnectionClosed()));

		
		ui->statusBar->showMessage( "Accepted connection" );
		if( _actAsProxyServer )
		{
			connectToTcpServer();
		}
    }
	else
	{
		ui->statusBar->showMessage( "Rejected 2nd connection" );
	}
}

void CulistGui::processServerConnectionData( )
{
    QByteArray data = _tcpServerConnection->readAll();
    traceDataReceived(data, "S<--");

	if ( _actAsProxyServer )
	{		
		send( data );
	}
	else if( !_dataSending && getSendSocket() == _tcpServerConnection )
	{		
		if ( ui->actionAlways_ACK->isChecked()  ) //simulate ACK only if not sending data myself
		{
			if( !(data.size() == 1 && data.at(0) == ASTM_EOT) )
			{
				QByteArray sdata;
				sdata.append( ASTM_ACK );
				traceDataSent(sdata, "S-->");
				_tcpServerConnection->write(sdata);
			}
		}
		else if ( ui->actionAlways_NAK->isChecked()  ) //simulate NAK only if not sending data myself
		{
			QByteArray data;
			data.append( ASTM_NAK );
			traceDataSent(data, "S-->");
			_tcpServerConnection->write(data);
		}		
	}
	else if( _dataSending )
	{
		if ( data.size() == 1 && data.at(0) == ASTM_ACK )
		{
			_currentSendAttempt=0;
		}
		else //if ( data.size() != 1 || data.at(0) == ASTM_NAK )
		{
			++_currentSendAttempt;
			_dataToSendCurIdx=0;			
		}		
		QTimer::singleShot(1, this, SLOT( processDataToSend() )  );
	}
}

void CulistGui::processServerConnectionError(QAbstractSocket::SocketError )
{
	if ( _actAsProxyServer )
		on_actionDisconnect_triggered();

    _tcpServerConnection->deleteLater();
    _tcpServerConnection=0;	
}

void CulistGui::processServerConnectionClosed(  )
{
	if ( _actAsProxyServer )
		on_actionDisconnect_triggered();

    _tcpServerConnection->deleteLater();
    _tcpServerConnection=0;

	ui->statusBar->showMessage( "Client disconnected" );
}



void CulistGui::send(const char *data)
{
    if(_tcpConnection)
    {
		traceDataSent(data, "C-->");
        _tcpConnection->write ( data );
    }
	else
		;//todo error
}




void CulistGui::onError( QAbstractSocket::SocketError )
{
   //_connected=false;
   //processDisconnectedState();
   on_actionDisconnect_triggered();
}



void CulistGui::on_actionAbout_Qt_activated()
{
  QApplication::aboutQt();
}


void CulistGui::on_actionLoad_Trace_triggered()
{
	QString tf = QFileDialog::getOpenFileName( this, tr("Select trace file") );
	loadTrace(tf);
}

bool CulistGui::loadTrace(QString tf)
{    
	if( !tf.isEmpty() )
	{
		QFile f(tf);
		if(!f.open(QIODevice::ReadOnly))
			return false;
		_lastTraceFile = tf;
		QTextStream ts(&f);
		while(!ts.atEnd())
		{
			QStringList sl = ts.readAll().split(QRegExp("[\n\r]"));
			//QStringList traceData;
			foreach( QString l, sl )
			{
				if( !l.isEmpty() )
				{
					//traceData << l;

					PAstm prec = ASTMFactory::instance().parse(l);

					if (!prec.isNull())
					{
						switch( prec->_type )
						{
							case EHeader:
								on_actionAdd_Session_triggered();
								on_actionAdd_Message_triggered();
//								on_actionAdd_Header_triggered();
								break;
/*							case EPatient:
								on_actionInsert_Patient_triggered();
								break;
							case EOrder:
								on_actionAdd_Order_triggered();
								break;
							case EResult:
								on_actionAdd_Result_triggered();
								break;
							case EComment:
								on_actionAdd_Comment_triggered();
								break;
							case ETerminator:
								on_actionAdd_Terminator_triggered();
								break;
*/
						}
						setRecord( prec );
					}
				}				
			}
			//_records.setStringList(traceData);
		}
	}
	return true;
}

void CulistGui::on_actionListen_triggered()
{

    if ( !_tcpServer.isListening() )
    {
		ui->actionStop_Listening->setEnabled(true);
		ui->actionListen->setEnabled(false);
		
		_tcpServer.listen(QHostAddress::Any, ui->eServerModeSrvPort->text().toUShort() );
        connect(&_tcpServer, SIGNAL(newConnection()),
             this, SLOT(acceptServerConnection()));
    }
}

void CulistGui::on_actionStop_Listening_triggered()
{
	if ( _tcpServer.isListening() )
    {
		ui->actionStop_Listening->setEnabled(false);
		ui->actionListen->setEnabled(true);
        disconnect(&_tcpServer, SIGNAL(newConnection()),
             this, SLOT(acceptServerConnection()));
		if (_tcpServerConnection)
		{
			_tcpServerConnection->close();
			_tcpServerConnection->deleteLater();
			_tcpServerConnection=0;
		}
        _tcpServer.close();
    }

}

void CulistGui::on_actionDisconnect_triggered()
{
	//if( _connected )
	{
		if ( _tcpConnection )
		{
			_tcpConnection->close();
			_tcpConnection->deleteLater();
			_tcpConnection=0;
		}
		ui->actionConnect->setEnabled(true);
		ui->actionDisconnect->setEnabled(false);
		ui->statusBar->showMessage( "Connection to Server closed" );
	}
}

void CulistGui::on_actionConnect_triggered()
{
	if( !_tcpConnection )
	{
		ui->actionConnect->setEnabled(false);
		ui->actionDisconnect->setEnabled(true);
		connectToTcpServer();
	}
}

void CulistGui::clearRecordEditView()
{
	foreach( QObject * ov, _recordEditViews )
	{
		ov->setObjectName("");
		ov->deleteLater();
	}
	_recordEditViews.clear();
}

void CulistGui::processCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous )
{
	processRecordSelected( current );
}

void CulistGui::on_trvEditRecords_activated( const QModelIndex & index )
{
	processRecordSelected( index );
}

void CulistGui::on_trvEditRecords_entered( const QModelIndex & index )
{
	processRecordSelected( index );
}


void CulistGui::on_trvEditRecords_clicked( const QModelIndex & index )
{
	processRecordSelected( index );
}

void CulistGui::processRecordSelected( const QModelIndex & index )
{
	_currentEditItem = index;
	if ( _currentEditItem.isValid() )
	{
		clearRecordEditView();

		QStandardItem *cur = _editRecords.itemFromIndex(_currentEditItem);
		if ( !cur )
			return;

		RecordType rt = static_cast<RecordType>(_currentEditItem.data(Qt::UserRole+1).toInt());
		Astm *_rec=0;
		switch(rt)
		{
			case EPatient:
				_rec = new ASTMPatient(1);
			break;
			case ERequest:
				_rec = new ASTMRequest(1);
			break;
			case EScientific:
				_rec = new ASTMScientific(1);
			break;
			case EOrder:
				_rec = new ASTMOrder(1);
			break;
			case EResult:
				_rec = new ASTMResult(1);
			break;
			case EComment:
				_rec = new ASTMComment(1);
			break;
			case EManufacturer:
				_rec = new ASTMManufacturer(1);
			break;
			case EHeader:
				_rec = new ASTMHeader("");
			break;
			case ETerminator:
				_rec = new ASTMTerminator(1);
			break;
		}

		if ( !_rec )
		{
			return;
		}
		
		char rectype=0;
		int fi=-1;
		foreach( QString fld, _rec->fields() )
		{
			

			fi++;
			
			if ( fld == "type" )
			{
				QString type = _rec->values()["type"].toString();
				rectype = type.at(0).toAscii();
				continue;
			}
			if ( fld == "seq" )
				continue;

			QHBoxLayout *hboxLayout = new QHBoxLayout;
			_recordEditViews.append( hboxLayout );
			//QSpacerItem *spacerItem = new QSpacerItem(;
			ui->ltRecordFlds->addLayout( hboxLayout);
			QLabel * fn= new QLabel(ASTMFactory::instance().userName("astm_E1394E97",rectype,fi) );
			fn->setMinimumWidth(150);
			hboxLayout->addWidget( fn );
			QLineEdit * fv = new QLineEdit;
			hboxLayout->addWidget( fv );
			fv->setObjectName(fld+"_myLisTraqDynamicEditView4711");
			//hboxLayout->addSpacerItem( spacerItem );
			fn->show();
			fv->show();
			_recordEditViews.append( fn );
			_recordEditViews.append( fv );
		}

		QMap<QString,QVariant> vals = cur->data(Qt::UserRole+2).toMap();		

		
		for( QMap<QString, QVariant>::const_iterator it = vals.begin(), end = vals.end(); it!=end; ++it )
		{
			QString fld = it.key();
			if ( fld == "type" || fld == "seq" )
				continue;
			

			
			QLineEdit * valEdit = this->findChild<QLineEdit*>( fld+"_myLisTraqDynamicEditView4711" );
			if (!valEdit)
				return;
			valEdit->setText( it.value().toString() );
		}
	}
}
/*
QMap<QString, QVariant> EditRecord::values()
{
	QMap<QString, QVariant> vals;
	if ( !_rec )
	{
		return vals;
	}
	
	foreach( QString fld, _rec->fields() )
	{
		if ( fld == "type" || fld == "seq" )
			continue;

		QLineEdit * valEdit = this->findChild<QLineEdit*>( fld );
		if (!valEdit)
			break;
		vals[fld] = valEdit->text();
	}
	
	return vals;

}
*/
void CulistGui::on_trvEditRecords_customContextMenuRequested(const QPoint &pos)
{
	_currentEditItem = ui->trvEditRecords->indexAt( pos );
	if (_currentEditItem.isValid())
	{
		int itemType = _currentEditItem.data(Qt::UserRole+1).toInt();
		QMenu *ctx = new QMenu(this);
		switch( itemType )
		{
		case ESession:
			ctx->addAction( ui->actionLoad_Trace );
			ctx->addAction( ui->actionSave_Trace );
			ctx->addAction( ui->actionAdd_Session );
			ctx->addAction( ui->actionAdd_Message );
			ctx->addAction( ui->actionSend_Data );
			break;
		case EMessage:
			ctx->addAction( ui->actionAdd_Header );
			ctx->addAction( ui->actionAdd_Patient );
			ctx->addAction( ui->actionAdd_Manufacturer );
			ctx->addAction( ui->actionAdd_Scientific );
			ctx->addAction( ui->actionAdd_Request );
			ctx->addAction( ui->actionAdd_Comment );
			ctx->addAction( ui->actionAdd_Terminator );
			ctx->addAction( ui->actionSend_Data );
			break;
		case ERequest:
		case EScientific:
				//TODO
			break;
		case EHeader:
			ctx->addAction( ui->actionAdd_Comment );			
			break;
		case EPatient:
			ctx->addAction( ui->actionAdd_Order );
			ctx->addAction( ui->actionAdd_Comment );			
			break;
		case EOrder:
			ctx->addAction( ui->actionAdd_Result );
			ctx->addAction( ui->actionAdd_Comment );			
			break;
		case EResult:
			ctx->addAction( ui->actionAdd_Comment );
			break;
		case EManufacturer:
			ctx->addAction( ui->actionAdd_Comment );
			break;
		case EComment:
			break;
		case ETerminator:
			break;
		}
		ctx->move(this->mapToGlobal(pos));
		ctx->move(ctx->x(),ctx->y() + ctx->height() );
		ctx->show();
	}
}

void CulistGui::on_actionAdd_Session_triggered()
{	
	_editRecords.insertRow( _editRecords.rowCount() );
	_editRecords.setData( _editRecords.index(_editRecords.rowCount()-1,0), ESession, Qt::UserRole+1 );
	int rc = _editRecords.rowCount(); 
	_editRecords.setData( _editRecords.index(rc-1,0), "Session" );
	_currentEditItem = _editRecords.index(_editRecords.rowCount()-1,0);

}

void CulistGui::on_actionAdd_Patient_triggered()
{
	addRecord( EPatient );
}
void CulistGui::on_actionAdd_Manufacturer_triggered()
{
	addRecord( EManufacturer );
}
void CulistGui::on_actionAdd_Request_triggered()
{
	addRecord( ERequest );
}
void CulistGui::on_actionAdd_Scientific_triggered()
{
	addRecord( EScientific );
}
void CulistGui::on_actionAdd_Order_triggered()
{
	addRecord( EOrder );
}
void CulistGui::on_actionAdd_Result_triggered()
{
	addRecord( EResult );
}
void CulistGui::on_actionAdd_Comment_triggered()
{
	addRecord( EComment );
}
void CulistGui::on_actionAdd_Header_triggered()
{
	addRecord( EHeader );
}
void CulistGui::on_actionAdd_Terminator_triggered()
{
	addRecord( ETerminator );
}

/*bool CulistGui::dataFromMessage( QStandardItem *mesg, QByteArray &outData )
{
	QList<QByteArray> outDataArray;
	if (!dataFromMessage( mesg, outDataArray ))
		return false;

	for (int i=0; i< outDataArray.size(); ++i)
	{		
		outData.append( outDataArray.at(i) );
		outData.append(&ASTM_CR,1);
	}

	return true;
}*/

bool CulistGui::dataFromMessage( QStandardItem *mesg, QList<QByteArray> &outData )
{
	QStandardItem *cur = mesg;
	int curr=0;

	Separators sep;
	int patientNum=1;
	int orderNum=1;
	int resultNum=1;
	int commentNum=1;
	int scientificNum=1;
	int requestNum=1;
	int manufacturerNum=1;
	
	
	QList< QPair<QStandardItem *, int>  > hOrder;
	
	hOrder.append(QPair<QStandardItem *, int>(cur,curr) );
	for ( cur = hOrder.last().first->child( curr++,0); hOrder.size()>0; cur = hOrder.last().first->child( curr++,0) )
	{
		if (cur==0)
		{
			cur = hOrder.last().first;
			curr=hOrder.last().second;
			hOrder.takeLast();
			if(hOrder.size()>0)
				continue;
			else
				break;
		}
		RecordType recType = static_cast<RecordType>(cur->data( Qt::UserRole+1 ).toInt());

		Astm * rec=0;
		switch(recType)
		{
		case EHeader:
			rec = new ASTMHeader("");
			patientNum=1;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case EPatient:
			rec = new ASTMPatient(patientNum);
			patientNum++;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case ERequest:
			rec = new ASTMRequest(requestNum);
			requestNum++;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case EScientific:
			rec = new ASTMScientific(scientificNum);
			scientificNum++;
			break;
		case EOrder:
			rec = new ASTMOrder(orderNum);
			orderNum++;
			resultNum=1;
			commentNum=1;
			break;
		case EResult:
			rec = new ASTMResult(resultNum);
			resultNum++;
			commentNum=1;
			break;
		case EComment:
			rec = new ASTMComment(commentNum);
			commentNum++;
			break;
		case EManufacturer:
			rec = new ASTMManufacturer(manufacturerNum);
			manufacturerNum++;
			break;
		case ETerminator:
			rec = new ASTMTerminator(1); //todo: could be something else then 1?
			break;	
		case EMessage:
			rec = new Astm(); //todo: handle fake record message
			break;	
		default:
			return false;
		}
		//rec->_sep = sep;
		QMap<QString, QVariant> fields = cur->data( Qt::UserRole+2 ).toMap();
		for(QMap<QString, QVariant>::iterator it = fields.begin(); it!=fields.end();++it)
		{
			if (it.key() == "seq" || it.key() == "type")
				continue;
			rec->setValue( it.key(), it.value().toString() );
		}
		//sep = rec->_sep;
		
		QByteArray data = rec->dataToSend();
		outData.append(data);
		//data.prepend( (char)('0'+seq) );
        //ui->tRecordContent->append( data );
		delete rec;
		rec=0;
		//seq++;
		//if ( seq>7 )
		//	seq=0;

		if ( cur->hasChildren() )
		{
			hOrder.append(QPair<QStandardItem *, int>(cur,curr) );
			curr=0;
		}
	}
	return true;
}

/*
void CulistGui::on_actionInsert_Session_triggered()
{	
	_editRecords.insertRow( _editRecords.rowCount() );
	_editRecords.setData( _editRecords.index(_editRecords.rowCount()-1,0), 1, Qt::UserRole+1 );
	_editRecords.setData( _editRecords.index(_editRecords.rowCount()-1,0), "Session" );
	_currentEditItem = _editRecords.index(_editRecords.rowCount()-1,0);

}

void CulistGui::on_actionInsert_Patient_triggered()
{
	insertRecord( EPatient );
}
void CulistGui::on_actionInsert_Order_triggered()
{
	insertRecord( EOrder );
}
void CulistGui::on_actionInsert_Result_triggered()
{
	insertRecord( EResult );
}
void CulistGui::on_actionInsert_Comment_triggered()
{
	insertRecord( EComment );
}
*/

bool CulistGui::setRecord( PAstm rec )
{
	if(rec.isNull())
		return false;
	QStandardItem * cur = addRecord( rec->_type );
	if(!cur)
		return false;
	QMap<QString, QVariant> fields = rec->values();//todo for dbg
	cur->setData( rec->values(), Qt::UserRole+2);

	return true;
}

QStandardItem * CulistGui::addRecord( RecordType rt )
{
	QStandardItem *cur = 0;
	if ( _currentEditItem.isValid() )  
	{
		
		if ( _currentEditItem.data(Qt::UserRole+1).toInt() == EMessage)
		{
			cur = _editRecords.itemFromIndex(_currentEditItem);
		}
		else if ( _currentEditItem.data(Qt::UserRole+1).toInt() >= EHeader)
		{
			switch( rt )
			{
				case EHeader:
				case EPatient:
				case ETerminator:
				case EManufacturer:
				case EScientific:
				case ERequest:
					while( _currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() != EMessage)
						_currentEditItem = _currentEditItem.parent();
					break;
				case EOrder:
					while( _currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() != EPatient)
						_currentEditItem = _currentEditItem.parent();
					break;
				case EResult:
					while( _currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() != EOrder)
						_currentEditItem = _currentEditItem.parent();
					break;

				case EComment:
					while( _currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() == EComment)
						_currentEditItem = _currentEditItem.parent();
					break;
			}
			cur = _editRecords.itemFromIndex(_currentEditItem);
		}

		if( cur )
		{
			QStandardItem *child = new QStandardItem(_recordNames[rt]);
			child->setData( rt, Qt::UserRole+1 );
			TRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._profile, rt );
			QMap<QString,QVariant> vals;
			foreach( PFieldInfo fi, recInfo )
			{
				vals[fi->_shortName] = fi->_stdValue;
			}
			child->setEditable(false);
			child->setData(vals,Qt::UserRole+2);
			cur->appendRow(child);
			_currentEditItem = _editRecords.indexFromItem(child);
			cur = child;
		}
	}
	return cur;
}

/*void CulistGui::insertRecord( RecordType rt )
{
	if ( _currentEditItem.isValid() )  
	{
		_currentEditItem = _currentEditItem.parent();
		if ( _currentEditItem.isValid() )  
		{
			addRecord( rt );
		}
	}
}*/

void CulistGui::on_actionAdd_Message_triggered()
{
	if (_currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() == ESession)
	{
		QStandardItem *child = new QStandardItem("Message");
		QStandardItem *cur = _editRecords.itemFromIndex(_currentEditItem);
		child->setData( EMessage, Qt::UserRole+1 );
		child->setEditable(false);
		cur->appendRow(child);
		_currentEditItem = _editRecords.indexFromItem(child);
	}
}

/*void CulistGui::on_actionInsert_Message_triggered()
{
	if ( _currentEditItem.isValid() )  
	{
		_currentEditItem = _currentEditItem.parent();
		if ( _currentEditItem.isValid() )  
		{
			on_actionAdd_Message_triggered();
		}
	}
}*/

void CulistGui::onDataRead()
{	
    QByteArray data = _tcpConnection->readAll();
	traceDataReceived(data, "C<--");
    if ( _actAsProxyServer && _tcpServerConnection )
    {
		traceDataSent(data,"C-->");
        _tcpServerConnection->write( data);
    }
	else if( !_dataSending  )
	{
		if ( ui->actionAlways_ACK->isChecked() && !_dataSending ) //simulate ACK only if not sending data myself
		{
			if( !(data.size() == 1 && data.at(0) == ASTM_EOT) )
			{
				QByteArray sdata;
				sdata.append( ASTM_ACK );
				traceDataSent(sdata, "C-->");
				_tcpConnection->write(sdata);
			}
		}
		else if ( ui->actionAlways_NAK->isChecked() && !_dataSending ) //simulate NAK only if not sending data myself
		{
			QByteArray data;
			data.append( ASTM_NAK );
			traceDataSent(data, "C-->");
			_tcpConnection->write(data);
		}
	}
	else
	{
		if ( data.size() == 1 && data.at(0) == ASTM_ACK )
		{
			_currentSendAttempt=0;
		}
		else //if ( data.size() != 1 || data.at(0) == ASTM_NAK )
		{
			++_currentSendAttempt;
			_dataToSendCurIdx=0;			
		}		
		QTimer::singleShot(1, this, SLOT( processDataToSend() )  );
	}

}

void CulistGui::traceDataSent( const QByteArray & data, QString dir )
{
	QString sdata = QString("%1 TX:[%2] ").arg(dir).arg(QDateTime::currentDateTime().toString("yyyyMMddThhmmss"));
	sdata += Helpers::dataToString(data);
    //_records.insertRows( _records.rowCount(), 1 );
    //_records.setData( _records.index( _records.rowCount()-1), sdata );
    ui->tRecordContent->append(sdata);
}

void CulistGui::traceDataReceived( const QByteArray & data, QString dir )
{
	QString sdata = QString("%1 RX:[%2] ").arg(dir).arg(QDateTime::currentDateTime().toString("yyyyMMddThhmmss"));
	sdata += Helpers::dataToString(data);
    //_records.insertRows( _records.rowCount(), 1 );
    //_records.setData( _records.index( _records.rowCount()-1), sdata );
    ui->tRecordContent->append(sdata);
}

///handles connection-related widgets
void CulistGui::processConnectedState()
{
    //ui->bClose->setEnabled(true);
    //ui->bConnect->setEnabled(false);
}

void CulistGui::processDisconnectedState()
{
    //ui->bClose->setEnabled(false);
    //ui->bConnect->setEnabled(true);
}

QList<QByteArray> CulistGui::currentData()
{
	QList<QByteArray> dataToSendNorm;
	while( _currentEditItem.isValid() && _currentEditItem.data(Qt::UserRole+1).toInt() != ESession)
						_currentEditItem = _currentEditItem.parent();
	if ( _currentEditItem.isValid() )  
	{
		QList<QByteArray> dataToSend;
		
		if ( _currentEditItem.data(Qt::UserRole+1).toInt() == ESession)
		{
			QStandardItem *msg = _editRecords.itemFromIndex(_currentEditItem);
			
			
			if ( dataFromMessage( msg, dataToSend ) )
			{
				//if( !_dataSending)//todo append session
				{
					
					int seq=1;				
					for (int i =0; i<dataToSend.size(); ++i)
					{
						QByteArray tmpdata = dataToSend.at(i);
						if (tmpdata.isEmpty())
							continue;
						
						int sidx=0;
						int eidx = 0; //tmpdata.size( ) > sidx+239 ? sidx+239 : tmpdata.size( )-1;
						do
						{
							eidx = tmpdata.size( ) > sidx+239 ? sidx+239 : tmpdata.size( )-1;
							QByteArray data;
							
							data.append( (char)('0'+seq) );
							data.append(tmpdata.mid( sidx, eidx-sidx ));
							sidx = eidx+1;
							data.append(ASTM_CR);
							if( data.size()<241 )
								data.append(ASTM_ETX);
							else
								data.append(ASTM_ETB);

							
							
							unsigned char cs = Helpers::checkSum(data);
							data.append( Helpers::binToHex( cs ) );	
							data.append( ASTM_CR );
							data.append( ASTM_LF );
							data.prepend( ASTM_STX );
							dataToSendNorm.append(data);
							seq++;
							if ( seq>7 )
								seq=0;
							

						}while( eidx < tmpdata.size( )-1 );
					}
					dataToSendNorm.prepend(  QByteArray( (const char *)&ASTM_ENQ, 1 )  );
					dataToSendNorm.append(  QByteArray( (const char *)&ASTM_EOT, 1 )  );
				}
			}
		}
		else
		{
			//todo error		
		}
	}
	return dataToSendNorm;
}

void CulistGui::on_actionRemove_Item_triggered()
{
	if ( _currentEditItem.isValid() )  
	{
		QModelIndex removeItem = _currentEditItem;
		if ( _currentEditItem.row()>0 )
			_currentEditItem = _editRecords.index( _currentEditItem.row()-1, 0, _currentEditItem.parent() );
		else if (_currentEditItem.parent().isValid())
			_currentEditItem = _currentEditItem.parent();
		else
			_currentEditItem=QModelIndex();

		_editRecords.removeRow(  removeItem.row(), removeItem.parent() );
		ui->trvEditRecords->selectionModel()->select(_currentEditItem, QItemSelectionModel::Rows);
	}
	on_trvEditRecords_clicked( _currentEditItem );
}

void CulistGui::on_actionSend_Data_triggered()
{
	if ( createSendData() )
		QTimer::singleShot(10,this, SLOT(processDataToSend()) );
}

bool CulistGui::createSendData()
{
	_messagesToSend.clear();
	if ( _currentEditItem.isValid() )  
	{
		QStandardItem *msg = _editRecords.itemFromIndex(_currentEditItem);
		QList<QByteArray> dataToSend;		
		if ( !dataFromMessage( msg, dataToSend ) )
			return false;


		if ( _currentEditItem.data(Qt::UserRole+1).toInt() == EMessage)
		{
			_messagesToSend.enqueue(dataToSend);			
		}
		else if ( _currentEditItem.data(Qt::UserRole+1).toInt() == ESession)
		{					
			int seq=1;					
			for (int i =0; i<dataToSend.size(); ++i)
			{
				TSendData lis01a2 = Helpers::toLIS01_A2( dataToSend.at(i), seq );
				//foreach ( const QByteArray & a, lis01a2 )
					_messagesToSend.enqueue( lis01a2 );				
			}
			_messagesToSend.prepend( TSendData()<< QByteArray( (const char *)&ASTM_ENQ, 1 )  );
			_messagesToSend.append( TSendData() << QByteArray( (const char *)&ASTM_EOT, 1 )  );
		}
		else
		{
			return false;
		}
	}
	return true;
}

QTcpSocket * CulistGui::getSendSocket(  )
{

	QTcpSocket * sendSocket= _tcpConnection;
	QString tracePrefix ="C-->";
	if ( sendSocket==0 )
	{
		tracePrefix ="S-->";
		sendSocket = _tcpServerConnection;
	}

	if ( sendSocket )
		sendSocket->setProperty("tracePrefix",tracePrefix);

	return sendSocket;
}

void CulistGui::processSendDataTimeout()
{
	_currentSendAttempt=0;
	_messagesToSend.clear();
	_dataToSend.clear();
	_dataToSendCurIdx=0;
	_dataSending=false;
}

void CulistGui::processDataToSend()
{
	if ( _currentSendAttempt > 3 )
	{
		_currentSendAttempt=0;
		_messagesToSend.clear();
		_dataToSendCurIdx=0;
		_dataSending=false;
		_sendDataTimer->stop();
		return;
	}

	_sendDataTimer->stop();
	_sendDataTimer->start();

	if ( _dataToSend.isEmpty() ) //first iteration
	{
		_dataToSendCurIdx=0;
		_dataSending=true;
		if (_messagesToSend.isEmpty())
			return;//Warning empty data
		_dataToSend = _messagesToSend.dequeue( );
	}

	if ( _dataToSendCurIdx < _dataToSend.size() )
	{
		//TOFO mote to: getSendSocket
		QTcpSocket * sendSocket = getSendSocket();
		if (sendSocket)
		{
			traceDataSent(_dataToSend.at(_dataToSendCurIdx), sendSocket->property("tracePrefix").toString() );
			sendSocket->write( _dataToSend.at(_dataToSendCurIdx) );			
		}
		else 
		{
			_messagesToSend.clear();
			_dataToSend.clear();
			return; //todo error
		}
		_dataToSendCurIdx++;
		//QTimer::singleShot(1,this, SLOT(processDataToSend()) );
	}
	else
	{
		_dataToSend.clear();
		if ( _messagesToSend.isEmpty() ) //FINISHED!
		{
			_sendDataTimer->stop();
			_dataSending=false;			
		}
		else
		{
			QTimer::singleShot(1,this, SLOT(processDataToSend()) );
		}
	}
}

void CulistGui::on_actionClear_All_triggered()
{
    clearMessages();
    clearLog();
	_currentEditItem = QModelIndex();
	ui->trvEditRecords->setModel( &_editRecords );
	connect( ui->trvEditRecords->selectionModel() ,SIGNAL( currentRowChanged ( QModelIndex,QModelIndex))
			, this, SLOT(processCurrentRowChanged ( QModelIndex,QModelIndex)) );
	_editRecords.setColumnCount(1);
	_editRecords.setHorizontalHeaderLabels( QStringList()<<tr("Messages") );
	clearRecordEditView();
}

void CulistGui::on_actionFork_triggered()
{
    QProcess::startDetached(qApp->arguments().at(0));
}

void CulistGui::on_bClearLog_clicked()
{
	ui->tRecordContent->clear();
}

void CulistGui::on_bSaveEditedRecord_clicked()
{
	if ( _currentEditItem.isValid() )
	{
		QStandardItem *cur = _editRecords.itemFromIndex(_currentEditItem);
		QMap<QString,QVariant> vals = cur->data(Qt::UserRole+2).toMap();
	
		foreach( QString fld, vals.keys() )
		{
			if ( fld == "type" || fld == "seq" )
				continue;

			QLineEdit * valEdit = this->findChild<QLineEdit*>( fld+"_myLisTraqDynamicEditView4711" );
			if (!valEdit)
				break;
			vals[fld] = valEdit->text();
		}
		cur->setData(vals,Qt::UserRole+2);
	}
}

void CulistGui::on_actionSave_Trace_triggered()
{
    if (_lastTraceFile.isEmpty())
    {
        _lastTraceFile = QFileDialog::getSaveFileName( this, tr("Select file name") );
        if (_lastTraceFile.isEmpty())
            return;
    }
    saveTrace(_lastTraceFile);
}

void CulistGui::saveTrace(QString fn)
{
    QFile of(fn);
	if(!of.open(QIODevice::WriteOnly))
		return; //todo error

	createSendData();
	while( !_messagesToSend.isEmpty() )
	{
		TSendData data= _messagesToSend.takeFirst();
		foreach(QByteArray ditem, data )
		{
			of.write(Helpers::dataToString(ditem, QStringList()<<"CRLF"<<"CR").toAscii());
		}
	}
}


void CulistGui::on_actionNew_Project_triggered()
{
	QString fn = QFileDialog::getSaveFileName( this, tr("Project file to save") );
    if (fn.isEmpty())
            return;
	_projectData._fn = fn;
}

void CulistGui::on_actionSave_Project_triggered()
{

    if (_projectData._fn.isEmpty())
            return;

	QFile pf( _projectData._fn );
	if ( pf.open(QIODevice::WriteOnly) )
	{
		QTextStream ts(&pf);

		ts << "ServerModePort="<<_projectData._serverModePort<<"\n";
		ts << "ClientModeServerPort="<<_projectData._clientModeServerPort<<"\n";
		ts << "ClientModeServer="<<_projectData._clientModeServer<<"\n";
		ts << "ProxyMode="<<_projectData._proxyMode<<"\n";
		ts << "Profile="<<_projectData._profile<<"\n";
		ts << "DATA\n";
		

		/*if ( _currentEditItem.data(Qt::UserRole+1).toInt() == EMessage)
		{
			ts << "Message=Message\n";
		}
		else if ( _currentEditItem.data(Qt::UserRole+1).toInt() == ESession)
		{					
			ts << "Session=Session\n";
		}
		else*/
		{
			int s=0;
			while ( QStandardItem *msg = _editRecords.item(s++,0) )
			{
				ts << "Session=Session\n";
				/*
				if ( _currentEditItem.data(Qt::UserRole+1).toInt() == EMessage)
				{
					ts << "Message=Message\n";
				}
				else if ( _currentEditItem.data(Qt::UserRole+1).toInt() == ESession)
				{					
					
				}*/

				QList<QByteArray> dataToSend;		
				if ( !dataFromMessage( msg, dataToSend ) )
				{
					return ;
				}
				foreach ( QByteArray d, dataToSend )
				{
					if(d.isEmpty())
						continue;

					if ( d.at(0) == 'H' )
						ts << "Message=Message\n";
					ts << d <<"\n";
				}
			}
		}
	}
	else
	{
		//TODO error
	}

}

void CulistGui::on_actionSave_Project_As_triggered()
{
	
}

void CulistGui::on_actionLoad_Project_triggered()
{
	QString fn = QFileDialog::getOpenFileName( this, tr("Select project file") );
	if (fn.isEmpty())
            return;

	_projectData._fn = fn;
    QFile pf( _projectData._fn );
	if ( pf.open(QIODevice::ReadOnly) )
	{
		QTextStream ts(&pf);
		on_actionClear_All_triggered();

		int ln=0;
		bool readData=false;
		while(!ts.atEnd())
		{
			++ln;
			QString l = ts.readLine();
			if(l.isEmpty() || l.at(0) == '#')
				continue;

			if ( readData )
			{				
				if (l.contains(QRegExp("\\s*Session\\s*=")) )
				{
					QString name = l.section("=",1);
					on_actionAdd_Session_triggered();
				}
				else if (l.contains(QRegExp("\\s*Message\\s*=")) )
				{
					QString name = l.section("=",1);
					on_actionAdd_Message_triggered();
				}
				else
				{
					PAstm prec = ASTMFactory::instance().parse(l);

					if (prec.isNull())
					{
						//TODO error
						return;
					}
					setRecord( prec );
				}
			}
			else if (l.contains(QRegExp("\\s*ServerModePort\\s*=")) )
				_projectData._serverModePort = l.section("=",1).toUShort();
			else if (l.contains(QRegExp("\\s*ClientModeServerPort\\s*=")) )
				_projectData._clientModeServerPort = l.section("=",1).toUShort();
			else if (l.contains(QRegExp("\\s*ClientModeServer\\s*=")) )
				_projectData._clientModeServer = l.section("=",1);
			else if (l.contains(QRegExp("\\s*ProxyMode\\s*=")) )
				_projectData._proxyMode = l.section("=",1).toInt();
			else if (l.contains(QRegExp("\\s*Profile\\s*=")) )
				_projectData._profile = l.section("=",1);
			else if (l=="DATA")
				readData=true;

		}
	
	}
	else
	{
		//TODO error
	}
}

void CulistGui::on_actionSave_Trace_As_triggered()
{
    QString fn = QFileDialog::getSaveFileName( this, tr("Select file name") );
    if (fn.isEmpty())
            return;
    _lastTraceFile=fn;
    saveTrace(_lastTraceFile);
}
