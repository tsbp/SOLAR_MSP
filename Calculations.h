//==============================================================================
#ifndef CALC_H
#define CALC_H
//==============================================================================
#include "LSM303.h"
//==============================================================================
#define FILTER_LENGHT   (35)
//==============================================================================
void addValueToArray(int aVal, int * aArr);
unsigned int mFilter(int * aBuf, unsigned int aLng);
void getAngles(u3AXIS_DATA* aRaw, double * aPitch, double * aRoll);
double heading(u3AXIS_DATA* aRawMag, double aPitch, double aRoll);
//==============================================================================
#endif