/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/

#include "qczmq.h"

///
//  Copy-construct to return the proper wrapped c types
QZconfig::QZconfig (zconfig_t *self, QObject *qObjParent) : QObject (qObjParent)
{
    this->self = self;
}


///
//  Create new config item
QZconfig::QZconfig (const QString &name, QZconfig *parent, QObject *qObjParent) : QObject (qObjParent)
{
    this->self = zconfig_new (name.toUtf8().data(), parent->self);
}

///
//  Load a config tree from a specified ZPL text file; returns a zconfig_t  
//  reference for the root, if the file exists and is readable. Returns NULL
//  if the file does not exist.                                             
QZconfig* QZconfig::load (const QString &filename, QObject *qObjParent)
{
    return new QZconfig (zconfig_load (filename.toUtf8().data()), qObjParent);
}

///
//  Equivalent to zconfig_load, taking a format string instead of a fixed
//  filename.                                                            
QZconfig* QZconfig::loadf (const QString &param, QObject *qObjParent)
{
    return new QZconfig (zconfig_loadf ("%s", param.toUtf8().data()), qObjParent);
}

///
//  Destroy a config item and all its children
QZconfig::~QZconfig ()
{
    zconfig_destroy (&self);
}

///
//  Return name of config item
const QString QZconfig::name ()
{
    const QString rv = QString (zconfig_name (self));
    return rv;
}

///
//  Return value of config item
const QString QZconfig::value ()
{
    const QString rv = QString (zconfig_value (self));
    return rv;
}

///
//  Insert or update configuration key with value
void QZconfig::put (const QString &path, const QString &value)
{
    zconfig_put (self, path.toUtf8().data(), value.toUtf8().data());
    
}

///
//  Equivalent to zconfig_put, accepting a format specifier and variable
//  argument list, instead of a single string value.                    
void QZconfig::putf (const QString &path, const QString &param)
{
    zconfig_putf (self, path.toUtf8().data(), "%s", param.toUtf8().data());
    
}

///
//  Get value for config item into a string value; leading slash is optional
//  and ignored.                                                            
const QString QZconfig::get (const QString &path, const QString &defaultValue)
{
    const QString rv = QString (zconfig_get (self, path.toUtf8().data(), defaultValue.toUtf8().data()));
    return rv;
}

///
//  Set config item name, name may be NULL
void QZconfig::setName (const QString &name)
{
    zconfig_set_name (self, name.toUtf8().data());
    
}

///
//  Set new value for config item. The new value may be a string, a printf  
//  format, or NULL. Note that if string may possibly contain '%', or if it 
//  comes from an insecure source, you must use '%s' as the format, followed
//  by the string.                                                          
void QZconfig::setValue (const QString &param)
{
    zconfig_set_value (self, "%s", param.toUtf8().data());
    
}

///
//  Find our first child, if any
QZconfig * QZconfig::child ()
{
    QZconfig *rv = new QZconfig (zconfig_child (self));
    return rv;
}

///
//  Find our first sibling, if any
QZconfig * QZconfig::next ()
{
    QZconfig *rv = new QZconfig (zconfig_next (self));
    return rv;
}

///
//  Find a config item along a path; leading slash is optional and ignored.
QZconfig * QZconfig::locate (const QString &path)
{
    QZconfig *rv = new QZconfig (zconfig_locate (self, path.toUtf8().data()));
    return rv;
}

///
//  Locate the last config item at a specified depth
QZconfig * QZconfig::atDepth (int level)
{
    QZconfig *rv = new QZconfig (zconfig_at_depth (self, level));
    return rv;
}

///
//  Execute a callback for each config item in the tree; returns zero if
//  successful, else -1.                                                
int QZconfig::execute (zconfig_fct handler, void *arg)
{
    int rv = zconfig_execute (self, handler, arg);
    return rv;
}

///
//  Add comment to config item before saving to disk. You can add as many
//  comment lines as you like. If you use a null format, all comments are
//  deleted.                                                             
void QZconfig::setComment (const QString &param)
{
    zconfig_set_comment (self, "%s", param.toUtf8().data());
    
}

///
//  Return comments of config item, as zlist.
QZlist * QZconfig::comments ()
{
    QZlist *rv = new QZlist (zconfig_comments (self));
    return rv;
}

///
//  Save a config tree to a specified ZPL text file, where a filename
//  "-" means dump to standard output.                               
int QZconfig::save (const QString &filename)
{
    int rv = zconfig_save (self, filename.toUtf8().data());
    return rv;
}

///
//  Equivalent to zconfig_save, taking a format string instead of a fixed
//  filename.                                                            
int QZconfig::savef (const QString &param)
{
    int rv = zconfig_savef (self, "%s", param.toUtf8().data());
    return rv;
}

///
//  Report filename used during zconfig_load, or NULL if none
const QString QZconfig::filename ()
{
    const QString rv = QString (zconfig_filename (self));
    return rv;
}

///
//  Reload config tree from same file that it was previously loaded from.
//  Returns 0 if OK, -1 if there was an error (and then does not change  
//  existing data).                                                      
int QZconfig::reload ()
{
    int rv = zconfig_reload (&self);
    return rv;
}

///
//  Load a config tree from a memory chunk
QZconfig * QZconfig::chunkLoad (QZchunk *chunk)
{
    QZconfig *rv = new QZconfig (zconfig_chunk_load (chunk->self));
    return rv;
}

///
//  Save a config tree to a new memory chunk
QZchunk * QZconfig::chunkSave ()
{
    QZchunk *rv = new QZchunk (zconfig_chunk_save (self));
    return rv;
}

///
//  Load a config tree from a null-terminated string
QZconfig * QZconfig::strLoad (const QString &string)
{
    QZconfig *rv = new QZconfig (zconfig_str_load (string.toUtf8().data()));
    return rv;
}

///
//  Save a config tree to a new null terminated string
QString QZconfig::strSave ()
{
    char *retStr_ = zconfig_str_save (self);
    QString rv = QString (retStr_);
    zstr_free (&retStr_);
    return rv;
}

///
//  Return true if a configuration tree was loaded from a file and that
//  file has changed in since the tree was loaded.                     
bool QZconfig::hasChanged ()
{
    bool rv = zconfig_has_changed (self);
    return rv;
}

///
//  Print the config file to open stream
void QZconfig::fprint (FILE *file)
{
    zconfig_fprint (self, file);
    
}

///
//  Print properties of object
void QZconfig::print ()
{
    zconfig_print (self);
    
}

///
//  Self test of this class
void QZconfig::test (bool verbose)
{
    zconfig_test (verbose);
    
}
/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/
