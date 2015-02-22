ODBCDNS - An ODBC-Based DNS Server
==================================

About
-----
ODBCDNS uses an RDBMS as its backend to store all of its information. This allows the use of powerful data manipulation characteristics seen in RDBMS systems. In addition, the cache remains intact when the program exits, but still maintains high levels of performance.

Requirements
------------
- 64-bit Windows
- ODBC 3 or newer
- Microsoft Visual Studio 2013
- Microsoft SQL Server 2008 or newer

Building
--------
To build, open MSVS solution file ODBCDNS.sln. From there, a build can be created simply by selecting a build configuration and building the solution.

Setup
-----
At a minimum, the database must be created and initialized with a RECORD table. This can be done by executing SQL/INIT.sql. It may also be useful to configure logging for the application using log4cxx. Please see http://logging.apache.org/log4cxx/usage.html for some example configurations. If this is not specified in the command line (see below), the default logger is used, which outputs to stdout.

Running
-------
To run ODBCNS from the command line, the following syntax is used:

```
usage:
        ODBCDNS.exe odbcconnectionstring [-option ...]

options:
        recursiveserver=SERVER  - server to use for recursive queries
        port=PORT               - port to listen to (default 53)
        threads=THREADS         - number of threads and ODBC connections to use (default 64)
        logproperties=FILE      - path to log4cxx properties file
```

Example: ODBCDNS.exe "DRIVER={SQL Server Native Client 11.0};SERVER=(local);DATABASE=DNS;Trusted_Connection=yes;" -recursiveserver=8.8.8.8 -logproperties=log4cxx.properties

Limitations
-----------
In its current state, ODBCDNS serves as a very simple caching DNS server. A few of its limitations are as follows:

- Only Microsoft SQL Server is supported at this time. Testing was done with a full version of SQL Server 2014. Other RDBMS systems that support the same syntax may work, but this has not been tested.
- Only Microsoft Windows is supported as a host OS. All testing was done with Windows 8.1 64-bit.
- Only A queries are supported. Other queries are forwarded to the recursive server, if specified.
- Only A and CNAME records are cached. Other records must be retrieved from the recursive server.
- The server only supports a recursive model through one specified server. It will not perform iterative lookups through the DNS tree.
- The server only serves as a caching server, and cannot be set up to be an authoritative server.
- Only DNS queries over UDP are supported.

Of course, these things may change as ODBCDNS evolves. :)
