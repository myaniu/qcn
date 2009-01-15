/* Platform independent version definitions... */

#ifndef QCN_VERSION_H
#define QCN_VERSION_H

/* Major part of QCN version number */
#define QCN_MAJOR_VERSION 4

/* Minor part of QCN version number */
#define QCN_MINOR_VERSION 30

/* Release part of QCN version number */
#define QCN_RELEASE 

/* String representation of QCN version number */
#define QCN_VERSION_STRING "4.30"

#if (defined(_WIN32) || defined(__APPLE__))
/* Name of package */
#define PACKAGE "qcn"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "QCN"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "QCN 4.30"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "qcn"

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.30"

#endif /* #if (defined(_WIN32) || defined(__APPLE__)) */

#endif /* #ifndef QCN_VERSION_H */

