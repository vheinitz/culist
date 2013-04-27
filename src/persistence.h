/* ===========================================================================
 * Copyright 2010: Valentin Heinitz, www.heinitz-it.de
 * Simple Persistence framework for Qt
 * Author: Valentin Heinitz, 2010-12-24
 * License: GPL, http://www.gnu.org/licenses/gpl.html
 *
 * D E S C R I P T I O N
 *   This framework is intended for storing persistent data of small Qt4
 * Applications easily.
 * It is nice, to have e.g. last used file loaded on a new
 * start or to see the application position/size as used last time. Unfortuna-
 * telly many small application - some prototypes or internal tools - rarely
 * offer such basic features. The reason often is the lack of time to implement
 * such "nice-to have" stuff. There are many things to be considered format,
 * place of data, unified load/store procedures, etc.
 *   This framework helps to use persistency with very minimal implementation-
 * overhead. These are the steps to do for using it:
 * 1. include "persistence.h"
 * 2. call once macro PERSISTENCE_INIT after QApplication has been created
 *    ( usually called in main )
 * 3. For each item to store call once macro PERSISTENT
 *    ( usually called in constructor body of the class containing this
 *     variable or property )
 * That's it. The data will be stored on exit and loaded on start.
 *
 * The macro PERSISTENCE_INIT has two parameters - company name and app name
 *  e.g.: PERSISTENCE_INIT( "My Company", "My App Name" )
 * The macro PERSISTENT has three parameters. There are three variants
 * at the moment, e.g:
 *   PERSISTENT("MyIntVar", &SomeIntMember, this )
 *   PERSISTENT("MyStrVar", &SomeQStringMember, this )
 *   PERSISTENT("MyProperty", this, "geometry")
 * The two first variants store a value of a int/QString member
 * The third variant stores the value of a QObject property (e.g. geometry)
 * The macro PERSISTENCE_SAVE could be called explicitely for triggering
 * storing of persistence items. However, Persistence connects to
 * QApplication::aboutToQuit and saves automatically on exit.
 ========================================================================== */


#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QPair>
#include <QCoreApplication>

#define PERSISTENT( a,b,c) Persistence::instance().declarePersistentItem((a),(b),(c));
#define PERSISTENCE_INIT( compName, appName ) Persistence::instance().init((compName),(appName));
#define PERSISTENCE_SAVE Persistence::instance().saveItems();

class Persistence : public QObject
{
Q_OBJECT
    Persistence(QObject *parent = 0);
    QMap<QString, int*> _intItems;
    QMap<QString, QString*> _strItems;
    QMap<QString, QPair<QString, QObject*> > _objItems;

public:
    static Persistence & instance()
    {
        static Persistence inst;
        return inst;
    }

    bool init( const QString & compName, const QString & appName );

    bool declarePersistentItem( const QString &tag, int *item, QObject * containingObject=0 );
    bool declarePersistentItem( const QString &tag, QString *item, QObject * containingObject=0 );
    bool declarePersistentItem( const QString &tag, QObject * item, const QString & propery );

public slots:
    void saveItems();

};

#endif // PERSISTENCE_H
