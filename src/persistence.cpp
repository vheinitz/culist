/* ===========================================================================
 * Copyright 2010: Valentin Heinitz, www.heinitz-it.de
 * Simple Persistence framework for Qt
 * Author: Valentin Heinitz, 2010-12-24
 * License: GPL, http://www.gnu.org/licenses/gpl.html
 *
 * D E S C R I P T I O N
 * Persistence implementation.
 ========================================================================== */
#include "persistence.h"
#include <QSettings>
#include <QDebug>

Persistence::Persistence(QObject *parent) :
    QObject(parent)
{

}

bool Persistence::init( const QString & compName, const QString & appName )
{
    QCoreApplication::setOrganizationDomain(compName);
    QCoreApplication::setApplicationName(appName);
    connect (qApp,SIGNAL(aboutToQuit()),this,SLOT(saveItems()));
    return true;
}

bool Persistence::declarePersistentItem( const QString &tag, int *item, QObject * containingObject )
{
    _intItems[tag] = item;
    QSettings s;
    if( s.contains(tag) )
    {
       *item = s.value(tag).toInt();
    }
    return true;
}

bool Persistence::declarePersistentItem( const QString &tag, QString *item, QObject * containingObject )
{
    _strItems[tag] = item;
    QSettings s;
    if( s.contains(tag) )
    {
       *item = s.value(tag).toString();
    }
    return true;
}

bool Persistence::declarePersistentItem( const QString &tag, QObject * item, const QString & prop )
{
    _objItems[tag] = qMakePair(prop,item);
    QSettings s;
    if( s.contains(tag) )
    {
       item->setProperty( prop.toAscii(), s.value(tag));
    }
    return true;
}

void Persistence::saveItems()
{
    QSettings s;
    foreach(QString tag, _intItems.keys())
    {
        s.setValue(tag, *_intItems[tag]);
    }

    foreach(QString tag, _strItems.keys())
    {
        s.setValue(tag, *_strItems[tag]);
    }

    foreach(QString tag, _objItems.keys())
    {
        s.setValue( tag, _objItems[tag].second->property(_objItems[tag].first.toAscii()) );
    }
}
