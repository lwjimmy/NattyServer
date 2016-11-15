/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/
#ifndef Q_ZPROC_H
#define Q_ZPROC_H

#include "qczmq.h"

class QT_CZMQ_EXPORT QZproc : public QObject
{
    Q_OBJECT
public:

    //  Copy-construct to return the proper wrapped c types
    QZproc (zproc_t *self, QObject *qObjParent = 0);

    //  Returns CZMQ version as a single 6-digit integer encoding the major
    //  version (x 10000), the minor version (x 100) and the patch.        
    static int czmqVersion ();

    //  Returns true if the process received a SIGINT or SIGTERM signal.
    //  It is good practice to use this method to exit any infinite loop
    //  processing messages.                                            
    static bool interrupted ();

    //  Returns true if the underlying libzmq supports CURVE security.
    static bool hasCurve ();

    //  Return current host name, for use in public tcp:// endpoints.
    //  If the host name is not resolvable, returns NULL.            
    static QString hostname ();

    //  Move the current process into the background. The precise effect     
    //  depends on the operating system. On POSIX boxes, moves to a specified
    //  working directory (if specified), closes all file handles, reopens   
    //  stdin, stdout, and stderr to the null device, and sets the process to
    //  ignore SIGHUP. On Windows, does nothing. Returns 0 if OK, -1 if there
    //  was an error.                                                        
    static void daemonize (const QString &workdir);

    //  Drop the process ID into the lockfile, with exclusive lock, and   
    //  switch the process to the specified group and/or user. Any of the 
    //  arguments may be null, indicating a no-op. Returns 0 on success,  
    //  -1 on failure. Note if you combine this with zsys_daemonize, run  
    //  after, not before that method, or the lockfile will hold the wrong
    //  process ID.                                                       
    static void runAs (const QString &lockfile, const QString &group, const QString &user);

    //  Configure the number of I/O threads that ZeroMQ will use. A good  
    //  rule of thumb is one thread per gigabit of traffic in or out. The 
    //  default is 1, sufficient for most applications. If the environment
    //  variable ZSYS_IO_THREADS is defined, that provides the default.   
    //  Note that this method is valid only before any socket is created. 
    static void setIoThreads (size_t ioThreads);

    //  Configure the number of sockets that ZeroMQ will allow. The default  
    //  is 1024. The actual limit depends on the system, and you can query it
    //  by using zsys_socket_limit (). A value of zero means "maximum".      
    //  Note that this method is valid only before any socket is created.    
    static void setMaxSockets (size_t maxSockets);

    //  Set network interface name to use for broadcasts, particularly zbeacon.    
    //  This lets the interface be configured for test environments where required.
    //  For example, on Mac OS X, zbeacon cannot bind to 255.255.255.255 which is  
    //  the default when there is no specified interface. If the environment       
    //  variable ZSYS_INTERFACE is set, use that as the default interface name.    
    //  Setting the interface to "*" means "use all available interfaces".         
    static void setBiface (const QString &value);

    //  Return network interface to use for broadcasts, or "" if none was set.
    static const QString biface ();

    //  Set log identity, which is a string that prefixes all log messages sent
    //  by this process. The log identity defaults to the environment variable 
    //  ZSYS_LOGIDENT, if that is set.                                         
    static void setLogIdent (const QString &value);

    //  Sends log output to a PUB socket bound to the specified endpoint. To   
    //  collect such log output, create a SUB socket, subscribe to the traffic 
    //  you care about, and connect to the endpoint. Log traffic is sent as a  
    //  single string frame, in the same format as when sent to stdout. The    
    //  log system supports a single sender; multiple calls to this method will
    //  bind the same sender to multiple endpoints. To disable the sender, call
    //  this method with a null argument.                                      
    static void setLogSender (const QString &endpoint);

    //  Enable or disable logging to the system facility (syslog on POSIX boxes,
    //  event log on Windows). By default this is disabled.                     
    static void setLogSystem (bool logsystem);

    //  Log error condition - highest priority
    static void logError (const QString &param);

    //  Log warning condition - high priority
    static void logWarning (const QString &param);

    //  Log normal, but significant, condition - normal priority
    static void logNotice (const QString &param);

    //  Log informational message - low priority
    static void logInfo (const QString &param);

    //  Log debug-level message - lowest priority
    static void logDebug (const QString &param);

    //  Self test of this class.
    static void test (bool verbose);

    zproc_t *self;
};
#endif //  Q_ZPROC_H
/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/
