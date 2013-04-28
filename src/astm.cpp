/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#include "astm.h"
#include <QVariant>

Separators  Astm::_sep;

QList<QByteArray> Helpers::toLIS01_A2( QByteArray tmpdata, int & seq )
{
	QList<QByteArray> dataToSendNorm;
	if (!tmpdata.isEmpty())	
	{
		int sidx=0;
		int eidx = 0; //tmpdata.size( ) > sidx+239 ? sidx+239 : tmpdata.size( )-1;
		do
		{
			eidx = tmpdata.size( );
			if( eidx > sidx+239 )
				eidx = sidx+239;

			QByteArray data;
			
			data.append( (char)('0'+seq) );
			data.append(tmpdata.mid( sidx, eidx-sidx ));
			sidx = eidx;
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
	return dataToSendNorm;
}

QByteArray Helpers::toLIS2_A2( QByteArray lis01_A2data)
{

	//<STX>X - 2 chars prefix
	//<ETX>XX<CR><LF> - 5 chars postfix
	if ( lis01_A2data.size() <= 2+5 )
		return QByteArray(); //tdo log error. invalide LIS01-A2 data

	QByteArray lis2 = lis01_A2data.mid( 2, lis01_A2data.size()-(2+5) );
	return  lis2;
}

void ASTMFactory::init()
{
	_profilesInfo["astm_E1394E97"] = PRecordsInfo( new QMap< char,PRecordInfo >() );
	/*
	typedef QSharedPointer<FieldInfo> PFieldInfo;
typedef QSharedPointer< QList<PFieldInfo> > PRecordInfo;
typedef QSharedPointer< QMap<char,PRecordInfo> > PRecordsInfo;
typedef QMap< QString, PRecordsInfo> TProfileInfo;
	*/
	(*_profilesInfo["astm_E1394E97"])['H'] = PRecordInfo( new QList<PFieldInfo> );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"7.1.1","type","ASTMRecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"7.1.2","delimeter","DelimiterDefinition") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"7.1.3","message_id","MessageControlID") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,4,0,"7.1.4","password","AccessPassword") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,5,0,"7.1.5","sender","SenderNameorID") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,6,0,"7.1.6","address","SenderStreetAddress") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,7,0,"7.1.7","reserved","ReservedField") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,8,0,"7.1.8","phone","SenderTelephoneNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,9,0,"7.1.9","caps","CharacteristicsofSender") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,10,0,"7.1.10","receiver","ReceiverID") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,11,0,"7.1.11","comments","Comments") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,12,0,"7.1.12","processing_id","ProcessingID") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,13,0,"7.1.13","version","VersionNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['H']->append( PFieldInfo(new FieldInfo(EFtScalar,14,0,"7.1.14","timestamp","Date/TimeofMessage") ) );

	//Standard patient definition
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"8.1.1","type","RecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"8.1.2","seq","SequenceNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"8.1.3","practice_id","PracticeAssignedPatientID") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,4,0,"8.1.4","laboratory_id","LaboratoryAssignedPatientID") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,5,0,"8.1.5","id","PatientID") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,6,0,"8.1.6","name","PatientName") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,7,0,"8.1.7","maiden_name","Mother’sMaidenName") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,8,0,"8.1.8","birthdate","Birthdate") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,9,0,"8.1.9","sex","PatientSex") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,10,0,"8.1.10","race","PatientRace-EthnicOrigin") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,11,0,"8.1.11","address","PatientAddress") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,12,0,"8.1.12","reserved","ReservedField") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,13,0,"8.1.13","phone","PatientTelephoneNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,14,0,"8.1.14","physician_id","AttendingPhysicianID") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,15,0,"8.1.15","special_1","SpecialField#1") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,16,0,"8.1.16","special_2","SpecialField#2") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,17,0,"8.1.17","height","PatientHeight") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,18,0,"8.1.18","weight","PatientWeight") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,19,0,"8.1.19","diagnosis","Patient’sKnownDiagnosis") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,20,0,"8.1.20","medication","Patient’sActiveMedication") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,21,0,"8.1.21","diet","Patient’sDiet") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,22,0,"8.1.22","practice_field_1","PracticeFieldNo.1") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,23,0,"8.1.23","practice_field_2","PracticeFieldNo.2") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,24,0,"8.1.24","admission_date","Admission/DischargeDates") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,25,0,"8.1.25","admission_status","AdmissionStatus") ) );
	(*_profilesInfo["astm_E1394E97"])['P']->append( PFieldInfo(new FieldInfo(EFtScalar,26,0,"8.1.26","location","Location") ) );

	//Standard order definition
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"9.4.1","type","RecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"9.4.2","seq","SequenceNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"9.4.3","sample_id","SpecimenID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,4,0,"9.4.4","instrument","InstrumentSpecimenID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,5,0,"9.4.5","test","UniversalTestID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,6,0,"9.4.6","priority","Priority") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,7,0,"9.4.7","created_at","Requested/OrderedDate/Time") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,8,0,"9.4.8","sampled_at","SpecimenCollectionDate/Time") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,9,0,"9.4.9","collected_at","CollectionEndTime") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,10,0,"9.4.10","volume","CollectionVolume") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,11,0,"9.4.11","collector","CollectorID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,12,0,"9.4.12","action_code","ActionCode") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,13,0,"9.4.13","danger_code","DangerCode") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,14,0,"9.4.14","clinical_info","RelevantInformation") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,15,0,"9.4.15","delivered_at","Date/TimeSpecimenReceived") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,16,0,"9.4.16","biomaterial","SpecimenDescriptor") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,17,0,"9.4.17","physician","OrderingPhysician") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,18,0,"9.4.18","physician_phone","Physician’sTelephone#") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,19,0,"9.4.19","user_field_1","UserFieldNo.1") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,20,0,"9.4.20","user_field_2","UserFieldNo.2") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,21,0,"9.4.21","laboratory_field_1","LaboratoryFieldNo.1") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,22,0,"9.4.22","laboratory_field_2","LaboratoryFieldNo.2") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,23,0,"9.4.23","modified_at","Date/TimeReported") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,24,0,"9.4.24","instrument_charge","InstrumentCharge") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,25,0,"9.4.25","instrument_section","InstrumentSectionID") ) );
	(*_profilesInfo["astm_E1394E97"])['O']->append( PFieldInfo(new FieldInfo(EFtScalar,26,0,"9.4.26","report_type","ReportType") ) );

	//Standard result definition
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"10.1.1","type","RecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"10.1.2","seq","SequenceNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"10.1.3","test","UniversalTestID") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,4,0,"10.1.4","value","DataorMeasurementValue") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,5,0,"10.1.5","units","Units") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,6,0,"10.1.6","references","ReferenceRanges") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,7,0,"10.1.7","abnormal_flag","ResultAbnormalFlags") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,8,0,"10.1.8","abnormality_nature","NatureofAbnormalTesting") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,9,0,"10.1.9","status","ResultsStatus") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,10,0,"10.1.10","norms_changed_at","DateofChangeinInstrument") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,11,0,"10.1.11","operator","OperatorIdentification") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,12,0,"10.1.12","started_at","Date/TimeTestStarted") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,13,0,"10.1.13","completed_at","Date/TimeTestComplete") ) );
	(*_profilesInfo["astm_E1394E97"])['R']->append( PFieldInfo(new FieldInfo(EFtScalar,14,0,"10.1.14","instrument","InstrumentIdentification") ) );

	//Standard comment definition
	(*_profilesInfo["astm_E1394E97"])['C']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"11.1.1","type","RecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['C']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"11.1.2","seq","SequenceNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['C']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"11.1.3","source","CommentSource") ) );
	(*_profilesInfo["astm_E1394E97"])['C']->append( PFieldInfo(new FieldInfo(EFtScalar,4,0,"11.1.4","data","CommentText") ) );
	(*_profilesInfo["astm_E1394E97"])['C']->append( PFieldInfo(new FieldInfo(EFtScalar,5,0,"11.1.5","ctype","CommentType") ) );

	//Standard termination definition
	(*_profilesInfo["astm_E1394E97"])['L']->append( PFieldInfo(new FieldInfo(EFtScalar,1,0,"13.1.1","type","RecordTypeID") ) );
	(*_profilesInfo["astm_E1394E97"])['L']->append( PFieldInfo(new FieldInfo(EFtScalar,2,0,"13.1.2","seq","SequenceNumber") ) );
	(*_profilesInfo["astm_E1394E97"])['L']->append( PFieldInfo(new FieldInfo(EFtScalar,3,0,"13.1.3","code","Terminationcode") ) );








}

PAstm ASTMFactory::parse( const QString & sdata )
{
	QString rec = sdata;
	rec.replace(QRegExp("[<][^>]*[>].*"),"");
	PAstm prec;

	if ( rec.isEmpty() )
		return prec;

	if( rec.at(0) == 'H'  )
	{
		if ( rec.length() < 5 )
			return prec;

		prec->_sep._fieldSep = rec.at(1).toAscii();
		prec->_sep._repeatSep = rec.at(2).toAscii();
		prec->_sep._componentSep = rec.at(3).toAscii();
		prec->_sep._escapeSep = rec.at(4).toAscii();
	}

	QStringList fields = rec.split(prec->_sep._fieldSep);
	if (!fields.isEmpty())
	{
		char rt = fields.at(0).at( fields.at(0).size()-1 ).toAscii();
		
		switch( rt )
		{
			case 'H':
				prec = PAstm(new ASTMHeader( prec->_sep.toString() ));
				break;
			case 'P':
				prec = PAstm(new ASTMPatient(1));
				break;
			case 'O':
				prec = PAstm(new ASTMOrder(1));
				break;
			case 'R':
				prec = PAstm(new ASTMResult(1));
				break;
			case 'C':
				prec = PAstm(new ASTMComment(1));
				break;
			case 'M':
				prec = PAstm(new ASTMManufacturer(1));
				break;
			case 'S':
				prec = PAstm(new ASTMScientific(1));
				break;
			case 'Q':
				prec = PAstm(new ASTMRequest(1));
				break;
			case 'L':
				prec = PAstm(new ASTMTerminator(1));
				break;
		}
	}
	if (!prec.isNull())
	{
		int startIdx=1; //Skip type
		if ( prec->_type == EHeader )
			startIdx=2; //Skip delimeter
		for( int i=startIdx; i<fields.size(); ++i )
		{
			prec->setValue( i, fields.at(i) );
		}
	}
	return prec;
}


Astm::Astm( int seq ):_seq(seq),_error(ErrNone)
{
	
}

Astm::~Astm(void)
{
}

/*QString Astm::toString(bool allFields)
{
	QString out;
	switch( _type )
	{
		case EHeader:
			out = "HEADER:";
			break;
		case EPatient:
			out = "PATIENT:";
			break;
		case EOrder:
			out = "ORDER:";
			break;
		case EResult:
			out = "RESULT:";
			break;
		case EComment:
			out = "COMMENT:";
			break;
		case ETerminator:
			out = "TERM:";
			break;
	}
	foreach ( QString n, _ent )
	{
		QString d = _vals[n];
		if ( !d.isEmpty() || allFields )
			out+="\t"+n+":"+d+"\n";
	}
	return out;
}*/

QMap<QString, QVariant> Astm::values()
{
	QMap<QString, QVariant> vals;
	foreach ( QString n, _ent )
	{
		if ( n == "seq" || n == "type")
			continue;

		vals.insert(n, QVariant(_vals[n]));
	}
	return vals;
}

bool Astm::constructRecord( QStringList  ent)
{
	_ent = ent;
	if ( _ent.contains("seq") )
		_vals["seq"] = QString::number(_seq);
	return true; //TODO may check for validness of entities
}

bool Astm::setValue( int idx, QString value)
{
	if ( idx<0 || idx >= _ent.size() )
		return false;
	setValue( _ent.at(idx), value );
	return true;
}

bool Astm::setValue( QString ent, QString value)
{
	if (ent=="type")
	{
		if (value.isEmpty())
			return false;

		//_type = value.at(0).toAscii();
		_vals[ent] = value;
		return true;
	}	
	else if (_type == EHeader && ent=="delimeter") //type head should be checked, but clear implicitly
	{
		setSeparators(value);
		_vals[ent] = value;
	}
	else if ( !_ent.contains(ent) )
		return false;
	else
		_vals[ent] = value;
}

QByteArray Astm::dataToSend( )
{
	QByteArray data;
	QByteArray nedata; //not empty
	bool first=true;
	bool second=false; // for skiping on H
	foreach( QString ent, _ent )
	{		
		if (!first)
		{
			if ( second && _type == EHeader )
			{		
				second = false;
			}
			else
			{
				nedata+=_sep._fieldSep;
			}
		}
		else
		{
			first=false;
			second=true;
		}

		if ( !_vals[ent].isEmpty() )
		{
			data +=nedata;
			nedata.clear();
			data+=_vals[ent].toAscii();		
		}
	}

	return data;
}


ASTMHeader::ASTMHeader(QString delim)
{
	constructRecord(QStringList() 
		<<"type"<<"delimeter"<<"message_id"<<"password"<<"sender"
		<<"address"<<"reserved"<<"phone"<<"caps"<<"receiver"<<"comments"
		<<"processing_id"<<"version"<<"timestamp" );
	_vals["type"] = "H";
	_type=EHeader;
	setSeparators( delim );
	_vals["delimeter"] = delim;
}

ASTMScientific::ASTMScientific( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"practice_id" <<"laboratory_id" <<"id" <<"name"
		<<"maiden_name" <<"birthdate" <<"sex" <<"race" <<"address" <<"reserved"
		<<"phone" <<"physician_id" <<"special_1" <<"special_2" <<"height" <<"weight"
		<<"diagnosis" <<"medication" <<"diet" <<"practice_field_1" 
		<<"practice_field_2" <<"admission_date" <<"admission_status" <<"location"
		);
	_vals["type"] = "S";
	_type=EScientific;
}

ASTMRequest::ASTMRequest( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"practice_id" <<"laboratory_id" <<"id" <<"name"
		<<"maiden_name" <<"birthdate" <<"sex" <<"race" <<"address" <<"reserved"
		<<"phone" <<"physician_id" <<"special_1" <<"special_2" <<"height" <<"weight"
		<<"diagnosis" <<"medication" <<"diet" <<"practice_field_1" 
		<<"practice_field_2" <<"admission_date" <<"admission_status" <<"location"
		);
	_vals["type"] = "Q";
	_type=ERequest;
}

ASTMPatient::ASTMPatient( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"practice_id" <<"laboratory_id" <<"id" <<"name"
		<<"maiden_name" <<"birthdate" <<"sex" <<"race" <<"address" <<"reserved"
		<<"phone" <<"physician_id" <<"special_1" <<"special_2" <<"height" <<"weight"
		<<"diagnosis" <<"medication" <<"diet" <<"practice_field_1" 
		<<"practice_field_2" <<"admission_date" <<"admission_status" <<"location"
		);
	_vals["type"] = "P";
	_type=EPatient;
}

ASTMOrder::ASTMOrder( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"spcmId" <<"instrSpcmId" <<"unvTestId" <<"priority" <<"created_at"
		<<"sampled_at" <<"collected_at" <<"volume" <<"collector" <<"action_code" <<"danger_code"
		<<"clinical_info" <<"delivered_at" <<"biomaterial" <<"physician" <<"physician_phone"
		<<"user_field_1" <<"user_field_2" <<"laboratory_field_1" <<"laboratory_field_2"
		<<"modified_at" <<"instrument_charge" <<"instrument_section" <<"report_type"
		);
	_vals["type"] = "O";
	_type=EOrder;
}

ASTMResult::ASTMResult( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"unvTestId" <<"value" <<"units"
		<<"references" <<"abnormal_flag" <<"abnormality_nature" <<"status"
		<<"norms_changed_at" <<"operator" <<"started_at" <<"completed_at" <<"instrument"
		);
	_vals["type"] = "R";
	_type=EResult;
}

ASTMComment::ASTMComment( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"source" <<"data" <<"ctype"
		);
	_vals["type"] = "C";
	_type=EComment;
}

ASTMManufacturer::ASTMManufacturer( int seq) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"mdf1"
			<<"mdf2"<<"mdf3"<<"mdf4"<<"mdf5"<<"mdf6"<<"mdf7"<<"mdf8"
			<<"mdf9"<<"mdf10"<<"mdf11"<<"mdf12"<<"mdf13"<<"mdf14"<<"mdf15"
			<<"mdf16"<<"mdf17"<<"mdf18"<<"mdf19"<<"mdf20"<<"mdf21"<<"mdf22"
		);
	_vals["type"] = "M";
	_type=EManufacturer;
}


ASTMTerminator::ASTMTerminator( int seq ) : Astm( seq )
{
	constructRecord( QStringList()
		<<"type" <<"seq" <<"code"
		);
	_vals["type"] = "L";
	_type=ETerminator;
}




ASTMMessage::ASTMMessage(void)
{
	_recNum=1;
	//_currentLevel=0;
	//_levelSeq<<1;
	_nextRecord = 0;
}

ASTMMessage::~ASTMMessage(void)
{
}

QByteArray ASTMMessage::dataToSend()
{
	if( _records.size() <= _nextRecord )
		return QByteArray();

	//int level = _records.at(_nextRecord)._level;
	QByteArray data = _records.at(_nextRecord)->dataToSend( );	
	data.prepend( '0'+_recNum );
	if ( ++_recNum > 7 )
		_recNum = 0;
	data.append( ASTM_CR );
	data.append( ASTM_ETX );
	unsigned char cs = Helpers::checkSum(data);
	data.append( Helpers::binToHex( cs ) );	
	data.append( ASTM_CR );
	data.append( ASTM_LF );
	data.prepend( ASTM_STX );
	

	return data;
}

bool ASTMMessage::atEnd()
{
	return( _records.size() <= _nextRecord );
}