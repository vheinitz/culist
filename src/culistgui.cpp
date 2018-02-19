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
#include <QToolBar>
#include <QMessageBox>
#include <QInputDialog>
#include "persistence.h"


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


	
	on_actionClear_All_triggered();
	
	ui->actionStop_Listening->setEnabled(false);
	ui->actionDisconnect->setEnabled(false);
	ui->actionSave_Project->setEnabled(false);
	ui->actionSave_Project_As->setEnabled(false);

	
	/*ui->lvProfileRecords->setModel(&_profileRecords);
	_profileRecords.setColumnCount(1);
	_profileRecords.setHorizontalHeaderLabels(QStringList()<<tr("Include/Exclude Record Names"));
	*/

	
	ASTMFactory::instance().init();
	ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() );
	setCurrentProfile( "ASTM_E1394_E97" );

	
	_sendDataTimer = new QTimer(this);
	_sendDataTimer->setSingleShot(true);
	_sendDataTimer->setInterval(3000);//TODO configurable
	connect( _sendDataTimer, SIGNAL(timeout()), this, SLOT(processSendDataTimeout()) );
	
	ui->trvEditProfile->setModel( &_profileFields );
	
	
	createToolBars();
	_winTitleBase = "CULIST. Version:"CULIST_VERSION"; Copyright 2013, Valentin Heinitz";
	this->setWindowTitle(_winTitleBase);

	
}


void CulistGui::setCurrentProfile( QString profile )
{
	//if set not in GUI
	int cnt = ui->cbCurrentProfile->count();
	for(int i=0; i<cnt; ++i)
	{
		if ( ui->cbCurrentProfile->itemText(i) == profile 
			&& ui->cbCurrentProfile->currentIndex() != i )
		{
				ui->cbCurrentProfile->setCurrentIndex(i);
			break;
		}
	}
	//end if set not in GUI
	
	_projectData._currentProfile = profile;
	_profileFields.clear();

	if ( !ASTMFactory::instance().profiles().contains( _projectData._currentProfile ) )
	{
		return;
	}
	if ( _projectData._currentProfile.isEmpty() )
	{
		return;
	}

	for ( TTypeToName::iterator it = ASTMFactory::instance()._recordNames.begin(); it != ASTMFactory::instance()._recordNames.end(); ++it )
	{
		if (profile != "ASTM_E1394_E97") //TODO remove this dev BP
			profile = profile;

		PRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._currentProfile,  it.key() );
		_profileFields.insertRow(_profileFields.rowCount());		
		_profileFields.setItem(_profileFields.rowCount()-1,0, new QStandardItem(it.value() ) );
		QStandardItem *curRec = _profileFields.item( _profileFields.rowCount()-1 ) ;
		curRec->setCheckable(true);
		curRec->setCheckState( recInfo->_visible ? Qt::Checked : Qt::Unchecked );
		curRec->setData(it.key(), Qt::UserRole+1);
				
		
		//int r=0;		
		for( QList<PFieldInfo>::const_iterator it = recInfo->constBegin(), end =  recInfo->constEnd(); it!=end; ++it  )
		{		
			PFieldInfo fi = *it;
			QString un = fi->_userName;
			QStandardItem *field = new QStandardItem();
			field->setCheckable(true);
			field->setCheckState( fi->_stdVisible ? Qt::Checked : Qt::Unchecked );
			field->setData( un, Qt::DisplayRole ); 
			curRec->appendRow( field );
			/*child->setData( rt, Qt::UserRole+1 );
			PRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._profile, rt );
			QMap<QString,QVariant> vals;
			foreach( PFieldInfo fi, recInfo.first )
			{
				vals[fi->_shortName] = fi->_stdValue;
			}
			child->setEditable(false);
			child->setData(vals,Qt::UserRole+2);
			cur->appendRow(child);
			_currentEditItem = _editRecords.indexFromItem(child);
			cur = child;

			/*_profileFields.setItem(r,0, new QStandardItem( fi->_userName ) );
			_profileFields.setItem(r,1, new QStandardItem( " " ) );
			_profileFields.item(r,1)->setCheckable(true);
			_profileFields.item(r,1)->setCheckState(fi->_stdVisible?Qt::Checked:Qt::Unchecked);
			_profileFields.setItem(r,2, new QStandardItem( fi->_stdValue ) );
			_profileFields.setItem(r,3, new QStandardItem( fi->_validation ) );
			_profileFields.setItem(r,4, new QStandardItem( fi->_isList ) );
			*/
			//++r;
		}
	}
}

CulistGui::~CulistGui()
{
    if (_tcpServerConnection)
		_tcpServerConnection->deleteLater();
	delete ui;

}


void CulistGui::createToolBars()
{
	_fileToolBar = addToolBar(tr("File"));    
	_fileToolBar->addAction(ui->actionNew_Project);
	_fileToolBar->addAction(ui->actionLoad_Project);
	_fileToolBar->addAction(ui->actionSave_Project);
	_fileToolBar->addAction(ui->actionSave_Project_As);
	_fileToolBar->addAction(ui->actionLoad_Trace);
	_fileToolBar->addAction(ui->actionExport);

	_editToolBar = addToolBar(tr("Edit"));    
	_editToolBar->addAction(ui->actionClear_All);
	_editToolBar->addAction(ui->actionAdd_Session);
	_editToolBar->addAction(ui->actionAdd_Message);
	_editToolBar->addAction(ui->actionAdd_Header);
	_editToolBar->addAction(ui->actionAdd_Patient);
	_editToolBar->addAction(ui->actionAdd_Order);
	_editToolBar->addAction(ui->actionAdd_Result);
	_editToolBar->addAction(ui->actionAdd_Request);
	_editToolBar->addAction(ui->actionAdd_Terminator);
	_editToolBar->addAction(ui->actionAdd_Comment);
	_editToolBar->addAction(ui->actionAdd_Scientific);
	_editToolBar->addAction(ui->actionAdd_Manufacturer);
	_editToolBar->addAction(ui->actionRemove_Item);

	_connectToolBar = addToolBar(tr("Connection"));    
	_connectToolBar->addAction(ui->actionConnect);
	_connectToolBar->addAction(ui->actionDisconnect);
	_connectToolBar->addAction(ui->actionListen);
	_connectToolBar->addAction(ui->actionStop_Listening);

	_runToolBar = addToolBar(tr("Run"));    
	_runToolBar->addAction(ui->actionSend_Data);
	_runToolBar->addAction(ui->actionAlways_ACK);
	_runToolBar->addAction(ui->actionAlways_NAK);
	_runToolBar->addAction(ui->actionFork);


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

void CulistGui::on_actionAbout_activated()
{
	QMessageBox::about(this,tr("About CULIST"),tr( 
		"<H1>CULIST</H1><p>Comprehensive, usable LIS tool.<br>"
		"Version: "CULIST_VERSION"<br>"
		"Copyright 2013, Valentin Heinitz, http://heinitz-it.de<br>"
		"License: Apache 2.0, http://www.apache.org/licenses/LICENSE-2.0<br>"
		"Source-code: https://code.google.com/p/culist<br>"
		));
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
	//processRecordSelected( current );
}

void CulistGui::on_trvEditRecords_activated( const QModelIndex & index )
{
	processRecordSelected( index, ui->cbCurrentProfile->currentText() );
}

void CulistGui::on_trvEditRecords_entered( const QModelIndex & index )
{
	processRecordSelected( index, ui->cbCurrentProfile->currentText() );
}


void CulistGui::on_trvEditRecords_clicked( const QModelIndex & index )
{
	processRecordSelected( index, ui->cbCurrentProfile->currentText() );
}

void CulistGui::processRecordSelected( const QModelIndex & index, const QString & profile )
{
	_currentEditItem = index;
	if ( _currentEditItem.isValid() )
	{
		clearRecordEditView();

		QStandardItem *cur = _editRecords.itemFromIndex(_currentEditItem);
		if ( !cur )
			return;

		RecordType rt = static_cast<RecordType>(_currentEditItem.data(Qt::UserRole+1).toInt());

		PRecordInfo recInfo = ASTMFactory::instance().recordInfo( profile, rt );


		if ( recInfo.isNull() )
		{
			return;
		}
		
		char rectype=0;
		//foreach( QString fld, recinfo->->fields() )
		for( QList<PFieldInfo>::const_iterator it = recInfo->constBegin(), end =  recInfo->constEnd(); it!=end; ++it  )
		{						
			if (  (*it)->_stdVisible )
			{
			QHBoxLayout *hboxLayout = new QHBoxLayout;
			_recordEditViews.append( hboxLayout );
			//QSpacerItem *spacerItem = new QSpacerItem(;
			ui->ltRecordFlds->addLayout( hboxLayout);
				QLabel * fn= new QLabel(ASTMFactory::instance().userName(profile,rt,(*it)->_recIdx) );
			fn->setMinimumWidth(150);
			hboxLayout->addWidget( fn );
			QLineEdit * fv = new QLineEdit;
			hboxLayout->addWidget( fv );
			fv->setObjectName((*it)->_shortName+"_myLisTraqDynamicEditView4711");
			//hboxLayout->addSpacerItem( spacerItem );
			fn->show();
			fv->show();
			_recordEditViews.append( fn );
			_recordEditViews.append( fv );
		}
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
			ctx->addAction( ui->actionExport );
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
			rec = new ASTMPatient();
			rec->setSeqNum( patientNum );
			patientNum++;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case ERequest:
			rec = new ASTMRequest();
			rec->setSeqNum( requestNum );
			requestNum++;
			orderNum=1;
			resultNum=1;
			commentNum=1;
			break;
		case EScientific:
			rec = new ASTMScientific();
			rec->setSeqNum( scientificNum );
			scientificNum++;
			break;
		case EOrder:
			rec = new ASTMOrder();
			rec->setSeqNum( orderNum );
			orderNum++;
			resultNum=1;
			commentNum=1;
			break;
		case EResult:
			rec = new ASTMResult();
			rec->setSeqNum( resultNum );
			resultNum++;
			commentNum=1;
			break;
		case EComment:
			rec = new ASTMComment();
			rec->setSeqNum( commentNum );
			commentNum++;
			break;
		case EManufacturer:
			rec = new ASTMManufacturer();
			rec->setSeqNum( manufacturerNum );
			manufacturerNum++;
			break;
		case ETerminator:
			rec = new ASTMTerminator(); //todo: could be something else then 1?
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
		//TODO implement	rec->setValue( it.key(), it.value().toString() );
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
	//QMap<QString, QVariant> fields = rec->values();//todo for dbg
	//cur->setData( rec->values(), Qt::UserRole+2);

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
			QStandardItem *child = new QStandardItem(ASTMFactory::instance()._recordNames[rt]);
			child->setData( rt, Qt::UserRole+1 );
			PRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._currentProfile, rt );
			QMap<QString,QVariant> vals;
			//foreach( PFieldInfo fi, recInfo.first )
			for( QList<PFieldInfo>::const_iterator it = recInfo->constBegin(), end =  recInfo->constEnd(); it!=end; ++it  )
			{
				vals[(*it)->_shortName] = (*it)->_stdValue;
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
		child->setEditable(true);
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

void CulistGui::updateRecordView()
{
/*
	for( int r = 0; r < _profileRecords.rowCount(); ++r )
	{		
		char recType = _profileRecords.item(r, 0)->data(Qt::UserRole+1 ).toChar().toAscii();
		bool isVisible = ASTMFactory::instance().isRecordVisible( _projectData._profile, recType );
		_profileRecords.item(r, 0)->setCheckState( isVisible?Qt::Checked:Qt::Unchecked);		
	}	
	*/
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
	_editRecords.setHorizontalHeaderLabels( QStringList()<<tr("Project's Messages") );
	clearRecordEditView();
}

void CulistGui::on_actionFork_triggered()
{
	//QString appExe = qApp->arguments().at(0);
	//TODO: pass arguments
	QProcess::startDetached(qApp->applicationFilePath(),QStringList(),qApp->applicationDirPath());
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
	QString fn = QFileDialog::getSaveFileName( this, tr("Define New Project's File") );
    if (fn.isEmpty())
            return;
	on_actionClear_All_triggered();	
	ui->cbCurrentProfile->clear();	
	ASTMFactory::instance().init();
	ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() ); //TODO: use model
	_projectData.reset();
	_projectData._fn = fn;
	setCurrentProfile( "ASTM_E1394_E97" );
	

	ui->actionSave_Project_As->setEnabled(true);
	ui->actionSave_Project->setEnabled(true);
	updateRecordView();

	
}

void CulistGui::on_actionSave_Project_triggered()
{
	if (_projectData._fn.isEmpty())
            return;
	saveProject();
}



void CulistGui::saveProject()
{   
 	QFile pf( _projectData._fn );
	if ( pf.open(QIODevice::WriteOnly) )
	{
		QTextStream ts(&pf);
		on_bSaveProfile_clicked();

		ts << "ServerModePort="<<_projectData._serverModePort<<"\n";
		ts << "ClientModeServerPort="<<_projectData._clientModeServerPort<<"\n";
		ts << "ClientModeServer="<<_projectData._clientModeServer<<"\n";
		ts << "ProxyMode="<<_projectData._proxyMode<<"\n";
		ts << "CurrentProfile="<<_projectData._currentProfile<<"\n";
		ts << ASTMFactory::instance().exportProfiles();
		ts << "DATA\n";
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
	QString fn = QFileDialog::getSaveFileName( this, tr("Define New Project's File") );
    if (fn.isEmpty())
            return;
	_projectData._fn = fn;
	saveProject();
}

void CulistGui::on_actionLoad_Project_triggered()
{
	QString fn = QFileDialog::getOpenFileName( this, tr("Select project file") );
	if (fn.isEmpty())
            return;
	
	on_actionClear_All_triggered();	
	ui->cbCurrentProfile->clear();	
	ASTMFactory::instance().init();
	_projectData.reset();
	_projectData._fn = fn;	

	ASTMFactory::instance().init();
    QFile pf( _projectData._fn );
	if ( pf.open(QIODevice::ReadOnly) )
	{
		QTextStream ts(&pf);
		on_actionClear_All_triggered();

		int ln=0;
		bool readData=false;
		bool readProfiles = false;
		while(!ts.atEnd())
		{
			++ln;
			QString l = ts.readLine();
			if(l.isEmpty() || l.at(0) == '#')
				continue;

			if (l=="DATA")
			{
				readData=true;
			}
			else if (l=="PROFILES BEGIN")
			{
				readData=false;
				readProfiles=true;
			}
			else if (l=="PROFILES END")
			{
				readData=false;
				readProfiles=false;
			}
			else if ( readProfiles )
			{	
				/*
				out  += QString( "%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n" )						
						.arg(pit.key())                    //prof. name  
						.arg(rit.key())					   // rec. name	
						.arg( rit.value().second ? 1 : 0 ) //Record is visible
						.arg((*fit)->_recIdx)			   // Field index
						.arg((*fit)->_isList?1:0)		   //Field is list			
						.arg((*fit)->_stdVisible)          //Visible
						.arg((*fit)->_stdValue)			   //Standard value
						.arg((*fit)->_validation)		   //Validation rule
				*/
				QStringList items = l.split("\t");//,QString::SkipEmptyParts);				
				if (items.size()==8)
				{
					QString profile = items.at(0);
					char recname = items.at(1).at(0).toAscii();
					bool recVisible = items.at(2).toInt();
					int fidx = items.at(3).toInt();
					bool isList = items.at(4).toInt();
					bool fVisible = items.at(5).toInt();
					QString stdVal = items.at(6);
					QString validRule = items.at(7);

					if ( !ASTMFactory::instance().profiles().contains(profile) )
					{
						ASTMFactory::instance().createProfile( profile );
					}
					ASTMFactory::instance().setRecordVisible( profile, recname, recVisible );

					ASTMFactory::instance().setFieldVisible( profile, recname, fidx, fVisible );

/*					ASTMFactory::instance().setFieldStdValue( _projectData._profile, _currentRt, r, 
						_profileFields.item( r, 2)->data(Qt::DisplayRole ).toString()
						);

					ASTMFactory::instance().setFieldValidator( _projectData._profile, _currentRt, r, 
						_profileFields.item( r, 3)->data(Qt::DisplayRole ).toString()
						);*/
				}
				
				
			}
			else if ( readData )
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
						//return;
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
			else if (l.contains(QRegExp("\\s*CurrentProfile\\s*=")) )
				_projectData._currentProfile = l.section("=",1);			

		}
		ui->actionSave_Project_As->setEnabled(true);
		ui->actionSave_Project->setEnabled(true);
		ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() );
		setCurrentProfile( _projectData._currentProfile );
		
	}
	else
	{
		//TODO error
	}
	updateRecordView();
}

void CulistGui::on_actionSave_Trace_As_triggered() //export trace
{

}

void CulistGui::on_actionExport_triggered()
{
    QString fn = QFileDialog::getSaveFileName( this, tr("Select file name") );
    if (fn.isEmpty())
            return;
    _lastTraceFile=fn;
    saveTrace(_lastTraceFile);
}

RecordType _currentRt=ESession;

/*
void CulistGui::on_lvProfileRecords_clicked(const QModelIndex &index)
{
	_profileFields.clear();
	_profileFields.setColumnCount(
		4
		);
	
	_profileFields.setHorizontalHeaderLabels( QStringList()
		<<tr("Field")<<tr("Used")<<tr("Std. value")<<tr("Validation")
		<<tr("Type")
	);

	ui->tvProfileFields->setModel( &_profileFields );

	QStandardItem *ri= _profileRecords.itemFromIndex(index);
	if (ri)
	{

		_currentRt = (RecordType)ri->data(Qt::UserRole+1).toInt();
		if ( ri->checkState() == Qt::Checked )
		{
			PRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._profile,  _currentRt );
			int r=0;
			foreach( PFieldInfo fi, recInfo.first )
			{
				_profileFields.insertRow(r);
				_profileFields.setItem(r,0, new QStandardItem( fi->_userName ) );
				_profileFields.setItem(r,1, new QStandardItem( " " ) );
				_profileFields.item(r,1)->setCheckable(true);
				_profileFields.item(r,1)->setCheckState(fi->_stdVisible?Qt::Checked:Qt::Unchecked);
				_profileFields.setItem(r,2, new QStandardItem( fi->_stdValue ) );
				_profileFields.setItem(r,3, new QStandardItem( fi->_validation ) );
				_profileFields.setItem(r,4, new QStandardItem( fi->_isList ) );
				++r;
			}
		}
	}
}
*/

void CulistGui::on_bExportProfile_clicked()
{

}

void CulistGui::on_bSaveProfile_clicked()
{
	if ( !_projectData._currentProfile.isEmpty() )
	{
		//_projectData._currentProfile = ui->cbCurrentProfile->currentText();
		for( int r = 0; r < _profileFields.rowCount(); ++r )
		{
			QStandardItem *recItem = _profileFields.item(r,0);
			ASTMFactory::instance().setRecordVisible( 
				_projectData._currentProfile, 
				recItem->data(Qt::UserRole+1).toChar().toAscii(),
				recItem->checkState() == Qt::Checked
				);
			int fidx=1;
			while( QStandardItem *fieldItem = recItem->child(fidx-1,0) )
			{
				ASTMFactory::instance().setFieldVisible( 
					_projectData._currentProfile, 
					recItem->data(Qt::UserRole+1).toChar().toAscii(), 
					fidx++, 
					fieldItem->checkState() == Qt::Checked
					);
			}
/*
			ASTMFactory::instance().setFieldStdValue( _projectData._profile, _currentRt, r, 
				_profileFields.item( r, 2)->data(Qt::DisplayRole ).toString()
				);

			ASTMFactory::instance().setFieldValidator( _projectData._profile, _currentRt, r, 
				_profileFields.item( r, 3)->data(Qt::DisplayRole ).toString()
				);				
				*/
		}
	}

/*
for ( TTypeToName::iterator it = ASTMFactory::instance()._recordNames.begin(); it != ASTMFactory::instance()._recordNames.end(); ++it )
	{
		_profileFields.insertRow(_profileFields.rowCount());		
		_profileFields.setItem(_profileFields.rowCount()-1,0, new QStandardItem(it.value() ) );
		QStandardItem *curRec = _profileFields.item( _profileFields.rowCount()-1 ) ;
		curRec->setCheckable(true);
		curRec->setData(it.key(), Qt::UserRole+1);
		PRecordInfo recInfo = ASTMFactory::instance().recordInfo( _projectData._profile,  it.key() );		
		
		int r=0;		
		foreach( PFieldInfo fi, recInfo.first )
		{		
			QString un = fi->_userName;
			QStandardItem *field = new QStandardItem();
			field->setCheckable(true);
			field->setData( un, Qt::DisplayRole ); 
			curRec->appendRow( field );
*/

	/*
	for( int r = 0; r < _profileRecords.rowCount(); ++r )
	{
		bool isUsed = _profileRecords.item(r, 0)->checkState() == Qt::Checked;
		char recType = _profileRecords.item(r, 0)->data(Qt::UserRole+1 ).toChar().toAscii();
		ASTMFactory::instance().setRecordVisible( _projectData._profile, recType, isUsed );
	}
	*/
}

void CulistGui::on_bCloneProfile_clicked()
{
	on_bSaveProfile_clicked();
	QString orig = ui->cbCurrentProfile->currentText();
	QString cloned = QInputDialog::getText(this,tr("Clone Profile"),tr("Input profile name"));
	if ( !cloned.isEmpty() )
	{
		ASTMFactory::instance().cloneProfile( orig, cloned );
		ui->cbCurrentProfile->clear();
		ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() );
		setCurrentProfile( cloned );
	}
}

void CulistGui::on_bNewProfile_clicked()
{
	on_bSaveProfile_clicked();
	QString newprof = QInputDialog::getText(this,tr("New Profile"),tr("Input profile name"));
	if ( !newprof.isEmpty() )
	{
		ASTMFactory::instance().createProfile( newprof );
		ui->cbCurrentProfile->clear();
		ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() );
		setCurrentProfile( newprof );
	}
}

void CulistGui::on_bDeleteProfile_clicked()
{
	QString remprof = ui->cbCurrentProfile->currentText();
	ASTMFactory::instance().removeProfile( remprof );
	ui->cbCurrentProfile->clear();
	ui->cbCurrentProfile->addItems( ASTMFactory::instance().profiles() );
	ui->cbCurrentProfile->setCurrentIndex(0);
	setCurrentProfile( ui->cbCurrentProfile->currentText() );
}

void CulistGui::on_cbCurrentProfile_currentIndexChanged(const QString &profile)
{
    on_bSaveProfile_clicked();
    setCurrentProfile( profile );
}

void CulistGui::on_bSendRaw_clicked()
{
	_sendRawBuffer = ui->tSendRaw->toPlainText().split("\n");
	sendRawBuffer();
}

void CulistGui::sendRawBuffer()
{
	if ( _sendRawBuffer.isEmpty() )
		return;

	QString sdata = _sendRawBuffer.takeFirst();

	if(sdata.indexOf("sleep")==0)//sleep 1000 -- 1 sec pause
	{
		QTimer::singleShot( sdata.section(" ",1).toInt(), this, SLOT(sendRawBuffer()));
		return;
	}


	sdata.replace("<CR>",QChar(ASTM_CR));
	sdata.replace("<LF>",QChar(ASTM_LF));
	sdata.replace("<ETB>",QChar(ASTM_ETB));
	sdata.replace("<STX>",QChar(ASTM_STX));
	sdata.replace("<ETX>",QChar(ASTM_ETX));
	sdata.replace("<ENQ>",QChar(ASTM_ENQ));
	sdata.replace("<EOT>",QChar(ASTM_EOT));
	sdata.replace("<ACK>",QChar(ASTM_ACK));
	sdata.replace("<NAK>",QChar(ASTM_NAK));

	//<STX>1H|\^&|||Helmed_IFA|||||||P|LIS2-A2|20130417180439<CR><ETB>1C<CR><LF>
	QByteArray data =  sdata.toLatin1();

    if(_tcpConnection)
    {
        traceDataSent(data, "C-->");
        _tcpConnection->write (data);
    }
    else if(_tcpServerConnection)
    {
        traceDataSent(data, "S-->");
        _tcpServerConnection->write ( data );
    }
    else
        _sendRawBuffer.clear();//todo error

	QTimer::singleShot( 1000, this, SLOT(sendRawBuffer()));
}