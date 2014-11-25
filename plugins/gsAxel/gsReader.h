// /////////////////////////////////////////////////////////////////
// Generated by axel-plugin wizard
// /////////////////////////////////////////////////////////////////

/* (C) MyCompany */


/* Put a short description of your plugin here */

/* MyCompany-contact@mycompany.com-http://www.mycompany.com */

#pragma once

#include "gsAxelPluginExport.h"

#include <dtkCore/dtkAbstractDataReader.h>

class GSAXELPLUGIN_EXPORT gsReader : public dtkAbstractDataReader
{
    Q_OBJECT
    
public :
    gsReader(void);
    ~gsReader(void);
    
public:
    QString identifier(void) const;
    QString description(void) const;
    QStringList handled(void) const;
    
    static bool registered(void);
    
public:

    bool canRead(const QString& file );
    bool read   (const QString& file );

};

dtkAbstractDataReader *creategsReader(void);
