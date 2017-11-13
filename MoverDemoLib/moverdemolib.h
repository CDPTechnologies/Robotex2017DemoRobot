/**
MoverDemoLib header file. Include this file in the project to use the library.
*/
#ifndef MOVERDEMOLIB_MOVERDEMOLIB_H
#define MOVERDEMOLIB_MOVERDEMOLIB_H

#include "MoverDemoLibBuilder.h"

namespace MoverDemoLib {

/** Instantiate the MoverDemoLibBuilder for this object */
MoverDemoLibBuilder gMoverDemoLibBuilder("MoverDemoLib", __TIMESTAMP__);

}

#endif // MOVERDEMOLIB_MOVERDEMOLIB_H
