#ifndef DEVICE_PRINTER_H
#define DEVICE_PRINTER_H

#ifdef BUILD_IEC
# include "iec/printer.h"
# include "iec/printerlist.h"
# define PRINTER_CLASS iecPrinter
#endif

#endif // DEVICE_PRINTER_H