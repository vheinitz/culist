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

//TODO handle codes
/*
S, P, R, C priority codes
L, H, <, >, N, U, D, B, W result codes
C, P, F, X, I, O result status
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



enum TFieldType{ 
	EFtScalar, ///<Simple type number or text
	ETfStruct, ///<Consists of sub-fields ( ^- deviced)
	ETfList,   ///<Consists of list ( \- deviced)
	ETfSub     ///<Subtype, implicitly scalar
};

class FieldInfo
{
public:
	TFieldType _type;       ///<Value type: scalar, array, structure, sub-type
	int _recIdx;			///<Index within the record
	int _fieldIdx;			///<Index within the field in case of subfield (^-separator)
	QString _stdRef;		///<Reference chapter in standard
	QString _shortName;		///<Short, uniq name for use in referencing via name
	QString _userName;		///<Full name as used in standard or defined by user (for custom fields)
	QString _info;			///<Documentation
	QString _validation;	///<Validation rules (rx)

	FieldInfo( 
		TFieldType type=EFtScalar
		, int recIdx=0
		, int fieldIdx=0
		, const QString & stdRef=QString::null
		, const QString & shortName=QString::null
		, const QString & userName=QString::null 
		, const QString & info=QString::null 
		, const QString & validation=QString::null ):
		_type(type),
		_recIdx(recIdx),
		_fieldIdx(fieldIdx),
		_stdRef(stdRef),
		_shortName(shortName),
		_userName(userName),
		_info(info),
		_validation(validation)
		{}
};

typedef QSharedPointer<FieldInfo> PFieldInfo;
typedef QSharedPointer< QList<PFieldInfo> > PRecordInfo;
typedef QSharedPointer< QMap<char,PRecordInfo> > PRecordsInfo;
typedef QMap< QString, PRecordsInfo> TProfileInfo;

class ASTMFactory
{
	ASTMFactory(){};
	TProfileInfo _profilesInfo;
	public:
		static ASTMFactory & instance()
		{
			static ASTMFactory inst;
			return inst;
		}
		void init();

		PAstm parse( const QString & sdata );
		QString userName( const QString & profile, char rt, int idx );
		QString stdRef( const QString & profile, char rt, int idx );
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
