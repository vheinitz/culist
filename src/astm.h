/*===========================================================================
# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0
# ==========================================================================*/

#ifndef ASMT_RECORD_H
#define ASMT_RECORD_H
#include <QMap>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QDateTime>
#include <QSharedPointer>



static const char ASTM_STX = 0x02;
static const char ASTM_ETX = 0x03;
static const char ASTM_ETB = 0x17;

static const char ASTM_ENQ = 0x05;
static const char ASTM_EOT = 0x04;

static const char ASTM_ACK = 0x06;
static const char ASTM_NAK = 0x15;

static const char ASTM_LF  = 0x0A;
static const char ASTM_CR  = 0x0D;

//TODO handle escape sequences
/*
&H& start highlighting text
&N& normal text (end highlighting)
&F& imbedded field delimiter character
&S& imbedded component field delimiter character
&R& imbedded repeat field delimiter character
&E& imbedded escape delimiter character
&Xhhhh& hexadecimal data
&Zcccc& Local (manufacturer defined) escape sequence
*/

typedef QMap<QString, QString> TRecordValues;

enum RecordType{
	ESession=1, 
	EMessage=2, 
	EHeader, 
	ERequest,
	EPatient,
	EOrder,
	EResult,
	EComment,
	EManufacturer,
	EScientific,
	ETerminator};


class Helpers
{
public:
	static QString binToHex( unsigned char b ) 
	{
		QString hex;
		static const char bin2Hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		hex += bin2Hex[  b >> 4 ];
		hex += bin2Hex[  b & 0xF ];
		return hex;
	}
	static QString timestamp()
	{
		return QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	}

	static QString dataToString(QByteArray ba, const QStringList & bl=QStringList() )
	{		
		QString d;
		for (int i=0; i<ba.size();++i)
		{
			switch (ba.at(i))
			{
				case ASTM_LF:
					d += "<LF>";
					if (bl.contains("CRLF"))
					{
						if (i>0 && ba.at(i-1) == '\r')
						d += "\n";
					}
					
					break;
				case ASTM_CR:
					d += "<CR>"; 
					if (bl.contains("CR"))
					{
						d += "\n";
					}
					break;
				case ASTM_EOT:
					d += "<EOT>"; 
					if (bl.contains("EOT"))
					{
						d += "\n";
					}
					break;
				case ASTM_STX:
					d += "<STX>"; break;
				case ASTM_ETX:
					d += "<ETX>"; break;
				case ASTM_ETB:
					d += "<ETB>"; break;
				case ASTM_ENQ:
					d += "<ENQ>"; 
					if (bl.contains("ENQ"))
					{
						d += "\n";
					}
					break;
				case ASTM_ACK:
					d += "<ACK>"; 
					if (bl.contains("ACK"))
					{
						d += "\n";
					}
					break;
				case ASTM_NAK:
					d += "<NAK>"; 
					if (bl.contains("NAK"))
					{
						d += "\n";
					}
					break;
				default:
				{
					if ( static_cast<unsigned char>(ba.at(i)) < 0x20)
					{
						d +="[";
						unsigned char b = static_cast<unsigned char>(ba.at(i));
						d += Helpers::binToHex(  b );
						d +="]";
					}
					else
						d+=ba.at(i);
				}
			}
		}
		return d;
	}

	static unsigned char checkSum( QByteArray data )
	{
		unsigned int cs=0;
		for(int i=0; i < data.size();++i)
		{
			cs+=static_cast<unsigned char>(data.at(i));
		}
		return (cs %256 ) & 0xFF;
	}

	static QList<QByteArray> toLIS01_A2( QByteArray lis2_A2data, int & seq );
	static QByteArray toLIS2_A2( QByteArray lis01_A2data);

};

struct Separators
{
	char _fieldSep;
	char _repeatSep;
	char _componentSep;
	char _escapeSep;
	//RecordType _type;
	Separators():
		_fieldSep('|'),
		_repeatSep('\\'),
		_componentSep('^'),
		_escapeSep('&')
	{}
	QString toString(  )
	{
		return QString("%1%2%3%4").arg(_fieldSep).arg(_repeatSep).arg(_componentSep).arg(_escapeSep);
	}
};

enum ARError{ErrNone, ErrUnexpectedRecord, ErrInvalideDelimiters};

class Astm : public QObject
{
protected:
	QStringList _ent;
	TRecordValues _vals;
	int _seq; //TODO: remove
	
public:
	static Separators _sep; //TODO: make not static - session/message context data
	void setSeparators( QString delim )
	{
		if (delim.size()<4)
			return;
		_sep._fieldSep = delim.at(0).toAscii();
		_sep._repeatSep = delim.at(1).toAscii();
		_sep._componentSep = delim.at(2).toAscii();
		_sep._escapeSep = delim.at(3).toAscii();
	}
	static const char _recordSep;
	RecordType _type;
	ARError _error;
	

	bool constructRecord( QStringList  ent);
	bool setValue( QString ent, QString value);
	bool setValue( int, QString value);
	bool isValide();
	QString toString(bool allFields=false);
	QByteArray dataToSend( );
	QMap<QString, QVariant> values();

	QStringList fields() {
		return _ent;
	}
	
	Astm( int seq=-1 );
	virtual ~Astm(void);
};



class ASTMHeader: public Astm
{
	public:
		ASTMHeader(QString delim);
};

class ASTMManufacturer: public Astm
{
	public:
		ASTMManufacturer( int seq);
};

class ASTMScientific: public Astm
{
	public:
		ASTMScientific( int seq);
};

class ASTMRequest: public Astm
{
	public:
		ASTMRequest( int seq);
};

class ASTMPatient: public Astm
{
	public:
		ASTMPatient( int seq);
};

class ASTMOrder: public Astm
{
	public:
		ASTMOrder( int seq);
};

class ASTMResult: public Astm
{
	public:
		ASTMResult( int seq);
};

class ASTMComment: public Astm
{
	public:
		ASTMComment( int seq);
};

class ASTMTerminator: public Astm
{
	public:
		ASTMTerminator(int);
};

typedef QSharedPointer<Astm> PAstm;

class ASTMParser
{
	ASTMParser();
	public:
		static ASTMParser & instance()
		{
			static ASTMParser inst;
			return inst;
		}

		PAstm parse( const QString & sdata );
};


class ASTMMessage : public QObject
{
	int _recNum;
	//int _currentLevel;
	//QList<int> _levelSeq;
	int _nextRecord;
	
public:
	QList <PAstm> _records;
	ASTMMessage(void);
	virtual ~ASTMMessage(void);
	QByteArray dataToSend();
	bool atEnd();

	void ack()
	{
		++_nextRecord;
	}

	void nack()
	{
		_nextRecord=0;
	}
};

#endif
/*
#: Header
#: +-----+--------------+---------------------------------+-------------------+
#: |  #  | ASTM Field # | ASTM Name                       | Python alias      |
#: +=====+==============+=================================+===================+
#: |   1 |        7.1.1 |             ASTM Record Type ID |              type |
#: +-----+--------------+---------------------------------+-------------------+
#: |   2 |        7.1.2 |            Delimiter Definition |         delimeter |
#: +-----+--------------+---------------------------------+-------------------+
#: |   3 |        7.1.3 |              Message Control ID |        message_id |
#: +-----+--------------+---------------------------------+-------------------+
#: |   4 |        7.1.4 |                 Access Password |          password |
#: +-----+--------------+---------------------------------+-------------------+
#: |   5 |        7.1.5 |               Sender Name or ID |            sender |
#: +-----+--------------+---------------------------------+-------------------+
#: |   6 |        7.1.6 |           Sender Street Address |           address |
#: +-----+--------------+---------------------------------+-------------------+
#: |   7 |        7.1.7 |                  Reserved Field |          reserved |
#: +-----+--------------+---------------------------------+-------------------+
#: |   8 |        7.1.8 |         Sender Telephone Number |             phone |
#: +-----+--------------+---------------------------------+-------------------+
#: |   9 |        7.1.9 |       Characteristics of Sender |              caps |
#: +-----+--------------+---------------------------------+-------------------+
#: |  10 |       7.1.10 |                     Receiver ID |          receiver |
#: +-----+--------------+---------------------------------+-------------------+
#: |  11 |       7.1.11 |                        Comments |          comments |
#: +-----+--------------+---------------------------------+-------------------+
#: |  12 |       7.1.12 |                   Processing ID |     processing_id |
#: +-----+--------------+---------------------------------+-------------------+
#: |  13 |       7.1.13 |                  Version Number |           version |
#: +-----+--------------+---------------------------------+-------------------+
#: |  14 |       7.1.14 |            Date/Time of Message |         timestamp |
#: +-----+--------------+---------------------------------+-------------------+

#: Patient Data
#: +-----+--------------+---------------------------------+-------------------+
#: |  #  | ASTM Field # | ASTM Name                       | Python alias      |
#: +=====+==============+=================================+===================+
#: |   1 |        8.1.1 |                  Record Type ID |              type |
#: +-----+--------------+---------------------------------+-------------------+
#: |   2 |        8.1.2 |                 Sequence Number |               seq |
#: +-----+--------------+---------------------------------+-------------------+
#: |   3 |        8.1.3 |    Practice Assigned Patient ID |       practice_id |
#: +-----+--------------+---------------------------------+-------------------+
#: |   4 |        8.1.4 |  Laboratory Assigned Patient ID |     laboratory_id |
#: +-----+--------------+---------------------------------+-------------------+
#: |   5 |        8.1.5 |                      Patient ID |                id |
#: +-----+--------------+---------------------------------+-------------------+
#: |   6 |        8.1.6 |                    Patient Name |              name |
#: +-----+--------------+---------------------------------+-------------------+
#: |   7 |        8.1.7 |            Mother’s Maiden Name |       maiden_name |
#: +-----+--------------+---------------------------------+-------------------+
#: |   8 |        8.1.8 |                       Birthdate |         birthdate |
#: +-----+--------------+---------------------------------+-------------------+
#: |   9 |        8.1.9 |                     Patient Sex |               sex |
#: +-----+--------------+---------------------------------+-------------------+
#: |  10 |       8.1.10 |      Patient Race-Ethnic Origin |              race |
#: +-----+--------------+---------------------------------+-------------------+
#: |  11 |       8.1.11 |                 Patient Address |           address |
#: +-----+--------------+---------------------------------+-------------------+
#: |  12 |       8.1.12 |                  Reserved Field |          reserved |
#: +-----+--------------+---------------------------------+-------------------+
#: |  13 |       8.1.13 |        Patient Telephone Number |             phone |
#: +-----+--------------+---------------------------------+-------------------+
#: |  14 |       8.1.14 |          Attending Physician ID |      physician_id |
#: +-----+--------------+---------------------------------+-------------------+
#: |  15 |       8.1.15 |                Special Field #1 |         special_1 |
#: +-----+--------------+---------------------------------+-------------------+
#: |  16 |       8.1.16 |                Special Field #2 |         special_2 |
#: +-----+--------------+---------------------------------+-------------------+
#: |  17 |       8.1.17 |                  Patient Height |            height |
#: +-----+--------------+---------------------------------+-------------------+
#: |  18 |       8.1.18 |                  Patient Weight |            weight |
#: +-----+--------------+---------------------------------+-------------------+
#: |  19 |       8.1.19 |       Patient’s Known Diagnosis |         diagnosis |
#: +-----+--------------+---------------------------------+-------------------+
#: |  20 |       8.1.20 |     Patient’s Active Medication |        medication |
#: +-----+--------------+---------------------------------+-------------------+
#: |  21 |       8.1.21 |                  Patient’s Diet |              diet |
#: +-----+--------------+---------------------------------+-------------------+
#: |  22 |       8.1.22 |            Practice Field No. 1 |  practice_field_1 |
#: +-----+--------------+---------------------------------+-------------------+
#: |  23 |       8.1.23 |            Practice Field No. 2 |  practice_field_2 |
#: +-----+--------------+---------------------------------+-------------------+
#: |  24 |       8.1.24 |       Admission/Discharge Dates |    admission_date |
#: +-----+--------------+---------------------------------+-------------------+
#: |  25 |       8.1.25 |                Admission Status |  admission_status |
#: +-----+--------------+---------------------------------+-------------------+
#: |  26 |       8.1.26 |                        Location |          location |
#: +-----+--------------+---------------------------------+-------------------+

ORDER
#: +-----+--------------+--------------------------------+--------------------+
#: |  #  | ASTM Field # | ASTM Name                      | Python alias       |
#: +=====+==============+================================+====================+
#: |   1 |        9.4.1 |                 Record Type ID |               type |
#: +-----+--------------+--------------------------------+--------------------+
#: |   2 |        9.4.2 |                Sequence Number |                seq |
#: +-----+--------------+--------------------------------+--------------------+
#: |   3 |        9.4.3 |                    Specimen ID |          sample_id |
#: +-----+--------------+--------------------------------+--------------------+
#: |   4 |        9.4.4 |         Instrument Specimen ID |         instrument |
#: +-----+--------------+--------------------------------+--------------------+
#: |   5 |        9.4.5 |              Universal Test ID |               test |
#: +-----+--------------+--------------------------------+--------------------+
#: |   6 |        9.4.6 |                       Priority |           priority |
#: +-----+--------------+--------------------------------+--------------------+
#: |   7 |        9.4.7 |    Requested/Ordered Date/Time |         created_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |   8 |        9.4.8 |  Specimen Collection Date/Time |         sampled_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |   9 |        9.4.9 |            Collection End Time |       collected_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |  10 |       9.4.10 |              Collection Volume |             volume |
#: +-----+--------------+--------------------------------+--------------------+
#: |  11 |       9.4.11 |                   Collector ID |          collector |
#: +-----+--------------+--------------------------------+--------------------+
#: |  12 |       9.4.12 |                    Action Code |        action_code |
#: +-----+--------------+--------------------------------+--------------------+
#: |  13 |       9.4.13 |                    Danger Code |        danger_code |
#: +-----+--------------+--------------------------------+--------------------+
#: |  14 |       9.4.14 |           Relevant Information |      clinical_info |
#: +-----+--------------+--------------------------------+--------------------+
#: |  15 |       9.4.15 |    Date/Time Specimen Received |       delivered_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |  16 |       9.4.16 |            Specimen Descriptor |        biomaterial |
#: +-----+--------------+--------------------------------+--------------------+
#: |  17 |       9.4.17 |             Ordering Physician |          physician |
#: +-----+--------------+--------------------------------+--------------------+
#: |  18 |       9.4.18 |        Physician’s Telephone # |    physician_phone |
#: +-----+--------------+--------------------------------+--------------------+
#: |  19 |       9.4.19 |               User Field No. 1 |       user_field_1 |
#: +-----+--------------+--------------------------------+--------------------+
#: |  20 |       9.4.20 |               User Field No. 2 |       user_field_2 |
#: +-----+--------------+--------------------------------+--------------------+
#: |  21 |       9.4.21 |         Laboratory Field No. 1 | laboratory_field_1 |
#: +-----+--------------+--------------------------------+--------------------+
#: |  22 |       9.4.22 |         Laboratory Field No. 2 | laboratory_field_2 |
#: +-----+--------------+--------------------------------+--------------------+
#: |  23 |       9.4.23 |             Date/Time Reported |        modified_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |  24 |       9.4.24 |              Instrument Charge |  instrument_charge |
#: +-----+--------------+--------------------------------+--------------------+
#: |  25 |       9.4.25 |          Instrument Section ID | instrument_section |
#: +-----+--------------+--------------------------------+--------------------+
#: |  26 |       9.4.26 |                    Report Type |        report_type |
#: +-----+--------------+--------------------------------+--------------------+

Result
#: +-----+--------------+--------------------------------+--------------------+
#: |  #  | ASTM Field # | ASTM Name                      | Python alias       |
#: +=====+==============+================================+====================+
#: |   1 |       10.1.1 |                 Record Type ID |               type |
#: +-----+--------------+--------------------------------+--------------------+
#: |   2 |       10.1.2 |                Sequence Number |                seq |
#: +-----+--------------+--------------------------------+--------------------+
#: |   3 |       10.1.3 |              Universal Test ID |               test |
#: +-----+--------------+--------------------------------+--------------------+
#: |   4 |       10.1.4 |      Data or Measurement Value |              value |
#: +-----+--------------+--------------------------------+--------------------+
#: |   5 |       10.1.5 |                          Units |              units |
#: +-----+--------------+--------------------------------+--------------------+
#: |   6 |       10.1.6 |               Reference Ranges |         references |
#: +-----+--------------+--------------------------------+--------------------+
#: |   7 |       10.1.7 |          Result Abnormal Flags |      abnormal_flag |
#: +-----+--------------+--------------------------------+--------------------+
#: |   8 |       10.1.8 |     Nature of Abnormal Testing | abnormality_nature |
#: +-----+--------------+--------------------------------+--------------------+
#: |   9 |       10.1.9 |                 Results Status |             status |
#: +-----+--------------+--------------------------------+--------------------+
#: |  10 |      10.1.10 | Date of Change in Instrument   |   norms_changed_at |
#: |     |              | Normative Values               |                    |
#: +-----+--------------+--------------------------------+--------------------+
#: |  11 |      10.1.11 |        Operator Identification |           operator |
#: +-----+--------------+--------------------------------+--------------------+
#: |  12 |      10.1.12 |         Date/Time Test Started |         started_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |  13 |      10.1.13 |        Date/Time Test Complete |       completed_at |
#: +-----+--------------+--------------------------------+--------------------+
#: |  14 |      10.1.14 |      Instrument Identification |         instrument |
#: +-----+--------------+--------------------------------+--------------------+

Comment
#: +-----+--------------+---------------------------------+-------------------+
#: |  #  | ASTM Field # | ASTM Name                       | Python alias      |
#: +=====+==============+=================================+===================+
#: |   1 |       11.1.1 |                  Record Type ID |              type |
#: +-----+--------------+---------------------------------+-------------------+
#: |   2 |       11.1.2 |                 Sequence Number |               seq |
#: +-----+--------------+---------------------------------+-------------------+
#: |   3 |       11.1.3 |                  Comment Source |            source |
#: +-----+--------------+---------------------------------+-------------------+
#: |   4 |       11.1.4 |                    Comment Text |              data |
#: +-----+--------------+---------------------------------+-------------------+
#: |   5 |       11.1.5 |                    Comment Type |             ctype |
#: +-----+--------------+---------------------------------+-------------------+

Term
#: +-----+--------------+---------------------------------+-------------------+
#: |  #  | ASTM Field # | ASTM Name                       | Python alias      |
#: +=====+==============+=================================+===================+
#: |   1 |       13.1.1 |                  Record Type ID |              type |
#: +-----+--------------+---------------------------------+-------------------+
#: |   2 |       13.1.2 |                 Sequence Number |               seq |
#: +-----+--------------+---------------------------------+-------------------+
#: |   3 |       13.1.3 |                Termination code |              code |
#: +-----+--------------+---------------------------------+-------------------+
*/