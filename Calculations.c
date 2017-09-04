//==============================================================================
#include "Calculations.h"
#include "math.h"
//==============================================================================

//==============================================================================
void addValueToArray(int aVal, int * aArr)
{
  for(unsigned int k = FILTER_LENGHT-1; k > 0; k--)
    aArr[k] = aArr[k-1];
  
  aArr[0] = aVal;
}
//==============================================================================
void getAngles(u3AXIS_DATA* aRaw, double * aPitch, double * aRoll)
{
  //===== roll, pitch ===========
  double cdf = 
    (double)aRaw->x * (double)aRaw->x + 
      (double)aRaw->y * (double)aRaw->y +
        (double)aRaw->z * (double)aRaw->z;      
  double accxnorm = (double)aRaw->x / sqrt(cdf);     
  double accynorm = (double)aRaw->y / sqrt(cdf);
  
  *aPitch = asin(-accxnorm);
  *aRoll =  asin(accynorm/cos(*aPitch));
  //====== yaw ======================
  
}
//==============================================================================
const int Mag_minx = -634;
const int Mag_miny = -761;
const int Mag_minz = -267;
const int Mag_maxx =  428;
const int Mag_maxy =  305;
const int Mag_maxz =  654;

//const int Mag_minx = -518;
//const int Mag_miny = -646;
//const int Mag_minz = -814;
//const int Mag_maxx =  674;
//const int Mag_maxy =  561;
//const int Mag_maxz =  334;
//==============================================================================
double heading(u3AXIS_DATA* aRawMag, double aPitch, double aRoll)
{
  double Magx = aRawMag->x;
  double Magy = aRawMag->y;
  double Magz = aRawMag->z;
  
//  Magx *= ((double)1100 / 1000);
//  Magy *= ((double)1100 / 1000);
//  Magz *= ((double)980  / 1000);

  // use calibration values to shift and scale magnetometer measurements
  Magx = (Magx-Mag_minx)/(Mag_maxx-Mag_minx)*2-1;
  Magy = (Magy-Mag_miny)/(Mag_maxy-Mag_miny)*2-1;
  Magz = (Magz-Mag_minz)/(Mag_maxz-Mag_minz)*2-1;
  
  double sinR = sin(aRoll);
  double cosR = cos(aRoll);
  double sinP = sin(aPitch);
  double cosP = cos(aPitch);
 

  // tilt compensated magnetic sensor measurements
  double magxcomp = Magx*cosP+Magz*sinP;
  double magycomp = Magx*sinR*sinP+Magy*cosR-Magz*sinR*cosP;
  
  // arctangent of y/x 
  //double Heading = (Math.atan2(magycomp,magxcomp));
  return (atan2(magycomp, magxcomp));
}
int avgBuf[FILTER_LENGHT];
//==============================================================================
unsigned int mFilter(int * aBuf, unsigned int aLng)
{  
#define MEDIAN
  
#ifdef MEDIAN
  
  for(unsigned int i = 0; i < aLng; i++) avgBuf[i] = aBuf[i];
  
  
  for(unsigned int k = 0; k < aLng - 1; k++)
    for(unsigned int i = k + 1; i < aLng - 1; i++)
    {
      if(avgBuf[k] < avgBuf[i])
      {
        unsigned int tmp =  avgBuf[k];
        avgBuf[k] = avgBuf[i];
        avgBuf[i] = tmp;
      }
    }
  return avgBuf[aLng >> 1];
#else 
  long sum = 0;
  for(unsigned int i = 0; i < aLng; i++)sum += aBuf[i];
  return sum / aLng; 
  
#endif
}
