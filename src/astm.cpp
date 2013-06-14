/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#include "astm.h"
#include <QVariant>
#include <QApplication>

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

#define tr(s) QString(s)

bool ASTMFactory::cloneProfile( const QString &orig, const QString &cloned )
{
	if (_profilesInfo.contains(orig))
	{
		TRecordsInfo opi = _profilesInfo[orig];
		_profilesInfo[ cloned ] = opi;
		return true;
	}
	return false;
}

void ASTMFactory::init()
{
	_recordNames.clear();
	_recordNames[EPatient] = tr("Patient");
	_recordNames[ERequest] = tr("Request");
	_recordNames[EScientific] = tr("Scientific");
	_recordNames[EOrder] = tr("Order");
	_recordNames[EResult] = tr("Result");
	_recordNames[EComment] = tr("Comment");
	_recordNames[EHeader] = tr("Header");
	_recordNames[ETerminator] = tr("Terminator");
	_recordNames[EManufacturer] = tr("Manufacturer");

	_profilesInfo.clear();
	_profilesInfo["ASTM_E1394_E97"]['H'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(1,0,"7.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(2,0,"7.1.2","delimeter","Delimiter Definition") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(3,0,"7.1.3","message_id","Message Control ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(4,0,"7.1.4","password","Access Password") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(5,0,"7.1.5","sender","Sender Nameor ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(6,0,"7.1.6","address","Sender Street Address") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(7,0,"7.1.7","reserved","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(8,0,"7.1.8","phone","Sender Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(9,0,"7.1.9","caps","Characteristics of Sender") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(10,0,"7.1.10","receiver","Receiver ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(11,0,"7.1.11","comments","Comments") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(12,0,"7.1.12","processing_id","Processing ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(13,0,"7.1.13","version","Version Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['H'].first.append( PFieldInfo(new FieldInfo(14,0,"7.1.14","timestamp","Date/Timeof Message") ) );

	//Standard patient definition
	_profilesInfo["ASTM_E1394_E97"]['P'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(1,0,"8.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(2,0,"8.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(3,0,"8.1.3","practice_id","Practice Assigned Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(4,0,"8.1.4","laboratory_id","Laboratory Assigned Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(5,0,"8.1.5","id","Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(6,0,"8.1.6","name","Patient Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(7,0,"8.1.7","maiden_name","Mother’s Maiden Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(8,0,"8.1.8","birthdate","Birthdate") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(9,0,"8.1.9","sex","Patient Sex") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(10,0,"8.1.10","race","Patient Race-, Ethnic Origin") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(11,0,"8.1.11","address","Patient Address") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(12,0,"8.1.12","reserved","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(13,0,"8.1.13","phone","Patient Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(14,0,"8.1.14","physician_id","Attending Physician ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(15,0,"8.1.15","special_1","Special Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(16,0,"8.1.16","special_2","Special Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(17,0,"8.1.17","height","Patient Height") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(18,0,"8.1.18","weight","Patient Weight") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(19,0,"8.1.19","diagnosis","Patient’s Known Diagnosis") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(20,0,"8.1.20","medication","Patient’s Active Medication") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(21,0,"8.1.21","diet","Patient’s Diet") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(22,0,"8.1.22","practice_field_1","Practice Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(23,0,"8.1.23","practice_field_2","Practice Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(24,0,"8.1.24","admission_date","Admission/Discharge Dates") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(25,0,"8.1.25","admission_status","Admission Status") ) );
	_profilesInfo["ASTM_E1394_E97"]['P'].first.append( PFieldInfo(new FieldInfo(26,0,"8.1.26","location","Location") ) );

	//Standard order definition
	_profilesInfo["ASTM_E1394_E97"]['O'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(1,0,"9.4.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(2,0,"9.4.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(3,0,"9.4.3","sample_id","Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(4,0,"9.4.4","instrument","Instrument Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(5,0,"9.4.5","test","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(6,0,"9.4.6","priority","Priority") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(7,0,"9.4.7","created_at","Requested/Ordered Date/Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(8,0,"9.4.8","sampled_at","Specimen Collection Date/Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(9,0,"9.4.9","collected_at","Collection End Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(10,0,"9.4.10","volume","Collection Volume") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(11,0,"9.4.11","collector","Collector ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(12,0,"9.4.12","action_code","Action Code") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(13,0,"9.4.13","danger_code","Danger Code") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(14,0,"9.4.14","clinical_info","Relevant Information") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(15,0,"9.4.15","delivered_at","Date/Time Specimen Received") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(16,0,"9.4.16","biomaterial","Specimen Descriptor") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(17,0,"9.4.17","physician","Ordering Physician") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(18,0,"9.4.18","physician_phone","Physician’s Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(19,0,"9.4.19","user_field_1","User Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(20,0,"9.4.20","user_field_2","User Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(21,0,"9.4.21","laboratory_field_1","Laboratory Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(22,0,"9.4.22","laboratory_field_2","Laboratory Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(23,0,"9.4.23","modified_at","Date/Time Reported") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(24,0,"9.4.24","instrument_charge","Instrument Charge") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(25,0,"9.4.25","instrument_section","Instrument Section ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O'].first.append( PFieldInfo(new FieldInfo(26,0,"9.4.26","report_type","Report Type") ) );

	//Standard result definition
	_profilesInfo["ASTM_E1394_E97"]['R'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(1,0,"10.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(2,0,"10.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(3,0,"10.1.3","test","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(4,0,"10.1.4","value","Data or Measurement Value") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(5,0,"10.1.5","units","Units") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(6,0,"10.1.6","references","Reference Ranges") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(7,0,"10.1.7","abnormal_flag","Result Abnormal Flags") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(8,0,"10.1.8","abnormality_nature","Nature of Abnormal Testing") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(9,0,"10.1.9","status","Results Status") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(10,0,"10.1.10","norms_changed_at","Date of Changein Instrument") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(11,0,"10.1.11","operator","Operator Identification") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(12,0,"10.1.12","started_at","Date/Time Test Started") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(13,0,"10.1.13","completed_at","Date/Time Test Complete") ) );
	_profilesInfo["ASTM_E1394_E97"]['R'].first.append( PFieldInfo(new FieldInfo(14,0,"10.1.14","instrument","Instrument Identification") ) );

	//Standard comment definition
	_profilesInfo["ASTM_E1394_E97"]['C'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['C'].first.append( PFieldInfo(new FieldInfo(1,0,"11.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['C'].first.append( PFieldInfo(new FieldInfo(2,0,"11.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['C'].first.append( PFieldInfo(new FieldInfo(3,0,"11.1.3","source","Comment Source") ) );
	_profilesInfo["ASTM_E1394_E97"]['C'].first.append( PFieldInfo(new FieldInfo(4,0,"11.1.4","data","Comment Text") ) );
	_profilesInfo["ASTM_E1394_E97"]['C'].first.append( PFieldInfo(new FieldInfo(5,0,"11.1.5","ctype","Comment Type") ) );

	//Standard request definition
	_profilesInfo["ASTM_E1394_E97"]['Q'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(1,0,"12.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(2,0,"12.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(3,0,"12.1.3","srangeid","Starting Range ID Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(4,0,"12.1.4","erangeid","Ending Range ID Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.5","utestid","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.6","noreqtmlim","Nature of Request Time Limits") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.7","begreqresdt","Beginning Request Results Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.8","endreqresdt","Ending Request Results Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.9","reqphysname","Requesting Physician Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.10","reqphystel","Requesting Physician Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.11","userfld1","User Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.12","userfld2","User Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q'].first.append( PFieldInfo(new FieldInfo(5,0,"12.1.13","utestid","Request Information Status Codes","","/[CPFXISMRANOD]/") ) );

	//Standard termination definition
	_profilesInfo["ASTM_E1394_E97"]['L'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['L'].first.append( PFieldInfo(new FieldInfo(1,0,"13.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['L'].first.append( PFieldInfo(new FieldInfo(2,0,"13.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['L'].first.append( PFieldInfo(new FieldInfo(3,0,"13.1.3","code","Termination Code") ) );

	//Standard manufacturer definition
	_profilesInfo["ASTM_E1394_E97"]['M'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['M'].first.append( PFieldInfo(new FieldInfo(1,0,"15.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['M'].first.append( PFieldInfo(new FieldInfo(2,0,"15.1.2","seq","Sequence Number") ) );

	//Standard scientific definition
	_profilesInfo["ASTM_E1394_E97"]['S'].second=true;
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(1,0,"14.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.3","anmeth","Analytical Method") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.4","instr","Instrumentation") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.5","reagents","Reagents") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.6","unitofmeas","Units of Measure") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.7","qc","Quality Control") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.8","spcmdescr","Specimen Descriptor") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.9","resrvd","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.10","container","Container") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.11","spcmid","Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.12","analyte","Analyte") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.13","result","Result") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.14","resunts","Result Units") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.15","collctdt","Collection Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.16","resdt","Result Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.17","anlprocstp","Analytical Preprocessing Steps") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.18","patdiagn","Patient Diagnosis") ) ); //TODO validate as IDC-9-CM
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.19","patbd","Patient Birthdate") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.20","patsex","Patient Sex") ) );
	_profilesInfo["ASTM_E1394_E97"]['S'].first.append( PFieldInfo(new FieldInfo(2,0,"14.1.21","patrace","Patient Race") ) );

}

QString ASTMFactory::userName( const QString & profile, char rt, int idx )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::ConstIterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value().first.size() > idx && idx>=0 )
				return riit.value().first.at(idx)->_userName;

	return QString::null;
}

QString ASTMFactory::stdRef( const QString & profile, char rt, int idx )
{
	return QString::null;
}

bool ASTMFactory::setFieldVisible( const QString & profile, char rt, int idx, bool visible )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value().first.size() > idx && idx>=0 )
			{
				riit->first.at(idx)->_stdVisible = visible;
				return true;
			}
			
	return false;
}

bool ASTMFactory::setFieldStdValue( const QString & profile, char rt, int idx, QVariant value )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value().first.size() > idx && idx>=0 )
			{
				riit->first.at(idx)->_stdValue = value.toString();
				return true;
			}
			
	return false;
}

bool ASTMFactory::setFieldValidator( const QString & profile, char rt, int idx, QString value )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value().first.size() > idx && idx>=0 )
			{
				riit->first.at(idx)->_validation = value;
				return true;
			}
			
	return false;
}

bool ASTMFactory::setRecordVisible( const QString & profile, char rt, bool visible )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
		{
			riit->second = visible;
			return true;
		}
			
	return false;
}


bool ASTMFactory::isRecordVisible( const QString & profile, char rt )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
	{
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
		{
			return riit->second;
		}
	}		

	return false; //TODO error message
}

QString ASTMFactory::exportProfiles( /*todo regexp or name*/ ) const
{
	QString out;
	for ( TProfileInfo::const_iterator pit = _profilesInfo.begin(), pend = _profilesInfo.end(); pit!=pend; ++pit )
	{
		out += QString("PROFILES BEGIN\n");
		for (TRecordsInfo::const_iterator rit = pit.value().constBegin(), rend =  pit.value().constEnd(); rit != rend; ++rit )
		{

			if(rit.value().second == true)// record visible
			{
				for (QList<PFieldInfo>::const_iterator fit = rit.value().first.constBegin(), fend =  rit.value().first.constEnd(); fit != fend; ++fit )
				{
					out  += QString( "%1\t%2\t%3\n" ).arg(pit.key()).arg(rit.key()).arg(rit.value().second)
						.arg((*fit)->_recIdx)
						.arg((*fit)->_isList?1:0)					
						.arg((*fit)->_stdVisible)
						.arg((*fit)->_stdValue)
						.arg((*fit)->_validation)
						;
				}
			}
		}
		out += QString("PROFILES END\n");
	}
	
	return out;
/*
	typedef QSharedPointer<FieldInfo> PFieldInfo;
typedef QPair< QList<PFieldInfo>,bool > TRecordInfo;
typedef QMap<char, TRecordInfo> TRecordsInfo;
typedef QMap< QString, TRecordsInfo> TProfileInfo;
*/

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
		//if ( n == "seq" || n == "type")
		//	continue;

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