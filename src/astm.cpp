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
	_profilesInfo["ASTM_E1394_E97"]['H'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(1,0,"7.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(2,0,"7.1.2","delimeter","Delimiter Definition") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(3,0,"7.1.3","message_id","Message Control ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(4,0,"7.1.4","password","Access Password") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(5,0,"7.1.5","sender","Sender Nameor ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(6,0,"7.1.6","address","Sender Street Address") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(7,0,"7.1.7","reserved","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(8,0,"7.1.8","phone","Sender Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(9,0,"7.1.9","caps","Characteristics of Sender") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(10,0,"7.1.10","receiver","Receiver ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(11,0,"7.1.11","comments","Comments") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(12,0,"7.1.12","processing_id","Processing ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(13,0,"7.1.13","version","Version Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['H']->append( PFieldInfo(new FieldInfo(14,0,"7.1.14","timestamp","Date/Timeof Message") ) );

	//Standard patient definition
	_profilesInfo["ASTM_E1394_E97"]['P'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(1,0,"8.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(2,0,"8.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(3,0,"8.1.3","practice_id","Practice Assigned Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(4,0,"8.1.4","laboratory_id","Laboratory Assigned Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(5,0,"8.1.5","id","Patient ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(6,0,"8.1.6","name","Patient Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(7,0,"8.1.7","maiden_name","Mother’s Maiden Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(8,0,"8.1.8","birthdate","Birthdate") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(9,0,"8.1.9","sex","Patient Sex") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(10,0,"8.1.10","race","Patient Race-, Ethnic Origin") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(11,0,"8.1.11","address","Patient Address") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(12,0,"8.1.12","reserved","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(13,0,"8.1.13","phone","Patient Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(14,0,"8.1.14","physician_id","Attending Physician ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(15,0,"8.1.15","special_1","Special Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(16,0,"8.1.16","special_2","Special Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(17,0,"8.1.17","height","Patient Height") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(18,0,"8.1.18","weight","Patient Weight") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(19,0,"8.1.19","diagnosis","Patient’s Known Diagnosis") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(20,0,"8.1.20","medication","Patient’s Active Medication") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(21,0,"8.1.21","diet","Patient’s Diet") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(22,0,"8.1.22","practice_field_1","Practice Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(23,0,"8.1.23","practice_field_2","Practice Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(24,0,"8.1.24","admission_date","Admission/Discharge Dates") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(25,0,"8.1.25","admission_status","Admission Status") ) );
	_profilesInfo["ASTM_E1394_E97"]['P']->append( PFieldInfo(new FieldInfo(26,0,"8.1.26","location","Location") ) );

	//Standard order definition
	_profilesInfo["ASTM_E1394_E97"]['O'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(1,0,"9.4.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(2,0,"9.4.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(3,0,"9.4.3","sample_id","Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(4,0,"9.4.4","instrument","Instrument Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(5,0,"9.4.5","test","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(6,0,"9.4.6","priority","Priority") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(7,0,"9.4.7","created_at","Requested/Ordered Date/Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(8,0,"9.4.8","sampled_at","Specimen Collection Date/Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(9,0,"9.4.9","collected_at","Collection End Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(10,0,"9.4.10","volume","Collection Volume") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(11,0,"9.4.11","collector","Collector ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(12,0,"9.4.12","action_code","Action Code") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(13,0,"9.4.13","danger_code","Danger Code") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(14,0,"9.4.14","clinical_info","Relevant Information") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(15,0,"9.4.15","delivered_at","Date/Time Specimen Received") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(16,0,"9.4.16","biomaterial","Specimen Descriptor") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(17,0,"9.4.17","physician","Ordering Physician") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(18,0,"9.4.18","physician_phone","Physician’s Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(19,0,"9.4.19","user_field_1","User Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(20,0,"9.4.20","user_field_2","User Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(21,0,"9.4.21","laboratory_field_1","Laboratory Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(22,0,"9.4.22","laboratory_field_2","Laboratory Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(23,0,"9.4.23","modified_at","Date/Time Reported") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(24,0,"9.4.24","instrument_charge","Instrument Charge") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(25,0,"9.4.25","instrument_section","Instrument Section ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['O']->append( PFieldInfo(new FieldInfo(26,0,"9.4.26","report_type","Report Type") ) );

	//Standard result definition
	_profilesInfo["ASTM_E1394_E97"]['R'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(1,0,"10.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(2,0,"10.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(3,0,"10.1.3","test","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(4,0,"10.1.4","value","Data or Measurement Value") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(5,0,"10.1.5","units","Units") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(6,0,"10.1.6","references","Reference Ranges") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(7,0,"10.1.7","abnormal_flag","Result Abnormal Flags") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(8,0,"10.1.8","abnormality_nature","Nature of Abnormal Testing") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(9,0,"10.1.9","status","Results Status") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(10,0,"10.1.10","norms_changed_at","Date of Changein Instrument") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(11,0,"10.1.11","operator","Operator Identification") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(12,0,"10.1.12","started_at","Date/Time Test Started") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(13,0,"10.1.13","completed_at","Date/Time Test Complete") ) );
	_profilesInfo["ASTM_E1394_E97"]['R']->append( PFieldInfo(new FieldInfo(14,0,"10.1.14","instrument","Instrument Identification") ) );

	//Standard comment definition
	_profilesInfo["ASTM_E1394_E97"]['C'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['C']->append( PFieldInfo(new FieldInfo(1,0,"11.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['C']->append( PFieldInfo(new FieldInfo(2,0,"11.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['C']->append( PFieldInfo(new FieldInfo(3,0,"11.1.3","source","Comment Source") ) );
	_profilesInfo["ASTM_E1394_E97"]['C']->append( PFieldInfo(new FieldInfo(4,0,"11.1.4","data","Comment Text") ) );
	_profilesInfo["ASTM_E1394_E97"]['C']->append( PFieldInfo(new FieldInfo(5,0,"11.1.5","ctype","Comment Type") ) );

	//Standard request definition
	_profilesInfo["ASTM_E1394_E97"]['Q'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(1,0,"12.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(2,0,"12.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(3,0,"12.1.3","srangeid","Starting Range ID Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(4,0,"12.1.4","erangeid","Ending Range ID Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.5","utestid","Universal Test ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.6","noreqtmlim","Nature of Request Time Limits") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.7","begreqresdt","Beginning Request Results Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.8","endreqresdt","Ending Request Results Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.9","reqphysname","Requesting Physician Name") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.10","reqphystel","Requesting Physician Telephone Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.11","userfld1","User Field No. 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.12","userfld2","User Field No. 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['Q']->append( PFieldInfo(new FieldInfo(5,0,"12.1.13","utestid","Request Information Status Codes","","/[CPFXISMRANOD]/") ) );

	//Standard termination definition
	_profilesInfo["ASTM_E1394_E97"]['L'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['L']->append( PFieldInfo(new FieldInfo(1,0,"13.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['L']->append( PFieldInfo(new FieldInfo(2,0,"13.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['L']->append( PFieldInfo(new FieldInfo(3,0,"13.1.3","code","Termination Code") ) );

	//Standard manufacturer definition
	_profilesInfo["ASTM_E1394_E97"]['M'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(1,0,"15.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.0.0","mf1","Manufacturer Field 1") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.0.0","mf2","Manufacturer Field 2") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.0.0","mf3","Manufacturer Field 3") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.0.0","mf4","Manufacturer Field 4") ) );
	_profilesInfo["ASTM_E1394_E97"]['M']->append( PFieldInfo(new FieldInfo(2,0,"15.0.0","mf5","Manufacturer Field 5") ) );

	//Standard scientific definition
	_profilesInfo["ASTM_E1394_E97"]['S'] = PRecordInfo( new TRecordInfo );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(1,0,"14.1.1","type","Record Type ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(2,0,"14.1.2","seq","Sequence Number") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(3,0,"14.1.3","anmeth","Analytical Method") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(4,0,"14.1.4","instr","Instrumentation") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(5,0,"14.1.5","reagents","Reagents") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(6,0,"14.1.6","unitofmeas","Units of Measure") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(7,0,"14.1.7","qc","Quality Control") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(8,0,"14.1.8","spcmdescr","Specimen Descriptor") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(9,0,"14.1.9","resrvd","Reserved Field") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(10,0,"14.1.10","container","Container") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(11,0,"14.1.11","spcmid","Specimen ID") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(12,0,"14.1.12","analyte","Analyte") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(13,0,"14.1.13","result","Result") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(14,0,"14.1.14","resunts","Result Units") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(15,0,"14.1.15","collctdt","Collection Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(16,0,"14.1.16","resdt","Result Date and Time") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(17,0,"14.1.17","anlprocstp","Analytical Preprocessing Steps") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(18,0,"14.1.18","patdiagn","Patient Diagnosis") ) ); //TODO validate as IDC-9-CM
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(19,0,"14.1.19","patbd","Patient Birthdate") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(20,0,"14.1.20","patsex","Patient Sex") ) );
	_profilesInfo["ASTM_E1394_E97"]['S']->append( PFieldInfo(new FieldInfo(21,0,"14.1.21","patrace","Patient Race") ) );

}

QString ASTMFactory::userName( const QString & profile, char rt, int fidx )
{
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::ConstIterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value()->size() > fidx && fidx>0 )
				return riit.value()->at(fidx)->_userName;

	return QString::null;
}

PFieldInfo ASTMFactory::fieldInfo( const QString & profile, char rt, int fidx )
{
	PFieldInfo result;
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value()->size() > fidx && fidx>=0 )
			{
				return riit.value()->at(fidx);
			}
	
	return result;
}


bool ASTMFactory::setFieldVisible( const QString & profile, char rt, int fidx, bool visible )
{

	if ( profile == "ASTM_E1394_E97" ) 
		return true; //TODO default not changeable. Make in a more efficient way!
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value()->size() < fidx || fidx<0 )
			{
				return false;
			}
			else
			{
				riit.value()->at(fidx)->_stdVisible = visible;
				return true;
			}
			
	return false;
}

bool ASTMFactory::setFieldStdValue( const QString & profile, char rt, int fidx, QVariant value )
{
	if ( profile == "ASTM_E1394_E97" ) 
		return true; //TODO default not changeable. Make in a more efficient way!
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value()->size() > fidx && fidx>0 )
			{
				riit.value()->at(fidx)->_stdValue = value.toString();
				return true;
			}
			
	return false;
}

bool ASTMFactory::setFieldValidator( const QString & profile, char rt, int fidx, QString value )
{
	if ( profile == "ASTM_E1394_E97" ) 
		return true; //TODO default not changeable. Make in a more efficient way!
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
			if ( riit.value()->size() > fidx && fidx>0 )
			{
				riit.value()->at(fidx)->_validation = value;
				return true;
			}
			
	return false;
}

bool ASTMFactory::setRecordVisible( const QString & profile, char rt, bool visible )
{
	if ( profile == "ASTM_E1394_E97" ) 
		return true; //TODO default not changeable. Make in a more efficient way!
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
		{
			riit.value()->_visible = visible;
			return true;
		}

	return false;
}

PRecordInfo ASTMFactory::recordInfo( const QString & profile, char rt )
{ 
	if ( !_profilesInfo.contains(profile) )
		return( PRecordInfo() );
	return _profilesInfo[profile][rt]; 
}

bool ASTMFactory::isRecordVisible( const QString & profile, char rt )
{
	if ( profile == "ASTM_E1394_E97" ) 
		return true; //TODO default not changeable. Make in a more efficient way!
	TProfileInfo::iterator prit = _profilesInfo.end();
	TRecordsInfo::Iterator riit;
	if ( (prit = _profilesInfo.find(profile)) != _profilesInfo.end()  )
	{
		if( (riit = prit->find( rt ) ) != prit.value().end()  )
		{
			return riit.value()->_visible;
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

			//if(rit.value().second == true)// record visible
			{
				for (QList<PFieldInfo>::const_iterator fit = rit.value()->constBegin(), fend =  rit.value()->constEnd(); fit != fend; ++fit )
				{
					out  += QString( "%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n" )						
						.arg(pit.key())                    //prof. name  
						.arg(rit.key())					   // rec. name	
						.arg( rit.value()->_visible ) //Record is visible
						.arg((*fit)->_recIdx)			   // Field index
						.arg((*fit)->_isList?1:0)		   //Field is list			
						.arg((*fit)->_stdVisible)          //Visible
						.arg((*fit)->_stdValue)			   //Standard value
						.arg((*fit)->_validation)		   //Validation rule
						;
				}
			}
		}
		out += QString("PROFILES END\n");
	}
	
	return out;
/*
	typedef QSharedPointer<FieldInfo> PFieldInfo;
typedef QPair< QList<PFieldInfo>,bool > PRecordInfo;
typedef QMap<char, PRecordInfo> TRecordsInfo;
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
				prec = PAstm(new ASTMPatient);
				break;
			case 'O':
				prec = PAstm(new ASTMOrder);
				break;
			case 'R':
				prec = PAstm(new ASTMResult);
				break;
			case 'C':
				prec = PAstm(new ASTMComment);
				break;
			case 'M':
				prec = PAstm(new ASTMManufacturer);
				break;
			case 'S':
				prec = PAstm(new ASTMScientific);
				break;
			case 'Q':
				prec = PAstm(new ASTMRequest);
				break;
			case 'L':
				prec = PAstm(new ASTMTerminator);
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


Astm::Astm( ):_error(ErrNone)
{
	
}

Astm::~Astm(void)
{
}


TRecordValues Astm::values()
{	
	return _vals;
}

/*bool Astm::constructRecord( QStringList  ent)
{
	_ent = ent;
	if ( _ent.contains("seq") )
		_vals["seq"] = QString::number(_seq);
	return true; //TODO may check for validness of entities
}*/

bool Astm::setValue( int idx, QString value, const QString & profile)
{
	if ( idx < 2 )
		return false;

	PFieldInfo fldInfo = ASTMFactory::instance().fieldInfo( profile, _type, idx );

	if( fldInfo.isNull() || !fldInfo->_stdVisible )
		return false;

	_vals[idx] = value;
	return true;
}

QByteArray Astm::dataToSend( )
{
	QByteArray data;
	int fidx=0;
	for( TRecordValues::const_iterator it = _vals.begin(), end = _vals.end(); it!=end; ++it  )
	{				
		for( int i=fidx; i <=it.key(); ++i )
		{
			data += Astm::_sep._fieldSep;
		}
		data+=it->toString().toAscii();		
		
		fidx++;
	}

	return data;
}


ASTMHeader::ASTMHeader(QString delim)
{
	_type=EHeader;
	_vals[0] = _type;
	_vals[1] = delim;
	setSeparators( delim );
}

ASTMScientific::ASTMScientific() : Astm(  )
{
	_type=EScientific;
	_vals[0] = _type;
}

ASTMRequest::ASTMRequest( ) : Astm(  )
{
	_type=ERequest;
	_vals[0] = _type;
}

ASTMPatient::ASTMPatient( ) : Astm(  )
{
	_type=EPatient;
	_vals[0] = _type;
}

ASTMOrder::ASTMOrder( ) : Astm(  )
{
	_type=EOrder;
	_vals[0] = _type;
}

ASTMResult::ASTMResult() : Astm(  )
{
	_type=EResult;
	_vals[0] = _type;
}

ASTMComment::ASTMComment( ) : Astm(  )
{
	_type=EComment;
	_vals[0] = _type;
}

ASTMManufacturer::ASTMManufacturer() : Astm(  )
{
	_type=EManufacturer;
	_vals[0] = _type;
}


ASTMTerminator::ASTMTerminator(  ) : Astm(  )
{
	_type=ETerminator;
	_vals[0] = _type;
}




ASTMMessage::ASTMMessage(void)
{
	_recNum=1;
	_nextRecord = 0;
}

ASTMMessage::~ASTMMessage(void)
{
}

QByteArray ASTMMessage::dataToSend()
{
	if( _records.size() <= _nextRecord )
		return QByteArray();

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