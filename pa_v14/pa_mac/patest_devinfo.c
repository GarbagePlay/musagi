/*
 * Test calling SPBSetDeviceInfo()
 * Should small parameters less than 4 bytes long be passed through a pointer, or directly?
 *
 * According to an Applet TechNote at:
 *     http://developer.apple.com/technotes/tn/tn1048.html
 *
 * ------------- begin quote --------------
 * Trouble with Void Pointers 
 * The SetSoundOutputInfo, SndSetInfo, and SPBSetDeviceInfo all take a void* as their last parameter.
 * Because the last parameter of these calls is a void*, it looks as if you need to pass a pointer
 * to the value you are setting, but this is not the case if what you are passing is 4 bytes or
 * less in size. If you are passing a parameter that is larger than four bytes, you must pass a
 * pointer to it.
 *
 * If you are calling GetSoundOutputInfo, SndGetInfo, or SPBGetDeviceInfo and
 * it is returning a value that is 4 bytes in size or less, it does not return a pointer to
 * that value, it just returns the value.
 * ------------ end quote ------------------
 * 
 * But my experiments on a PowerComputing Mac clone running Mac OS 8.1 indicate that
 * the TechNote is wrong and that a pointer should be passed to SPBSetDeviceInfo.
 *
 * Author: Phil Burk
 * Copyright 2000 Phil Burk
 * This code may be used freely for any purpose.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

/* Mac specific includes */
#include "OSUtils.h"
#include <MacTypes.h>
#include <Math64.h>
#include <Errors.h>
#include <Sound.h>
#include <SoundInput.h>
#include <Devices.h>
#include <DateTimeUtils.h>
#include <Timer.h>
#include <Gestalt.h>

/* Change this to 0 or 1 to use the direct parameter method, or the indirect pointer method. */
#define USE_POINTER    (1)

/* Debugging output macros. */
#define PRINT(x) { printf x; fflush(stdout); }
#define ERR_RPT(x) PRINT(x)
#define DBUG(x)   PRINT(x)

static int TestSettingDeviceInfo( const unsigned char *deviceName  );

int main( void );
int main( void )
{
	unsigned char noname = 0;
	int result = TestSettingDeviceInfo( (const unsigned char *) &noname );

	PRINT(("Test complete. result = %d\n", result ));
	return 0;
}

/*************************************************************************
**	TestSettingDeviceInfo()
**  Query information about a named input device.
*/
static int TestSettingDeviceInfo( const unsigned char *deviceName )
{
	OSErr err;
	long  mRefNum = 0;
	long  tempL;
	short tempS;
	Fixed tempF;
	
	err = SPBOpenDevice( deviceName, siWritePermission, &mRefNum);
	if (err)
	{
		PRINT(("Cound not open device!\n"));
		return -1;
	}
	
/* Define macros for printing out device info. */
#define PrintDeviceInfo(selector,var) \
	err = SPBGetDeviceInfo(mRefNum, selector, (Ptr) &var); \
	if (err) { \
		PRINT(("query %s failed\n", #selector )); \
	}\
	else { \
		PRINT(("query %s = 0x%x\n", #selector, var )); \
	}
	
	PrintDeviceInfo( siContinuous, tempS );
	PrintDeviceInfo( siAsync, tempS );
	PrintDeviceInfo( siNumberChannels, tempS );
	PrintDeviceInfo( siSampleSize, tempS );
	PrintDeviceInfo( siSampleRate, tempF );
	PrintDeviceInfo( siChannelAvailable, tempS );
	PrintDeviceInfo( siActiveChannels, tempL );
	PrintDeviceInfo( siDeviceBufferInfo, tempL );

#if USE_POINTER
/* Continuous Mode ---------- */
	PRINT(("Attempt to set siContinuous to 1 using pointer method.\n"));
	tempS = 1;
	err = SPBSetDeviceInfo(mRefNum, siContinuous, (Ptr) &tempS);
	if (err) PRINT(("setting siContinuous using pointer failed\n"));
	PrintDeviceInfo( siContinuous, tempS );

	PRINT(("Attempt to set siContinuous to 0 using pointer method.\n"));
	tempS = 1;
	err = SPBSetDeviceInfo(mRefNum, siContinuous, (Ptr) &tempS);
	if (err) PRINT(("setting siContinuous using pointer failed\n"));
	PrintDeviceInfo( siContinuous, tempS );
#else	
	PRINT(("Attempt to set siContinuous to 1 using direct method.\n"));
	err = SPBSetDeviceInfo(mRefNum, siContinuous, (Ptr) 1);
	if (err) PRINT(("setting siContinuous using direct failed\n"));
	PrintDeviceInfo( siContinuous, tempS );
	
	PRINT(("Attempt to set siContinuous to 0 using direct method.\n"));
	err = SPBSetDeviceInfo(mRefNum, siContinuous, (Ptr) 0);
	if (err) PRINT(("setting siContinuous using direct failed\n"));
	PrintDeviceInfo( siContinuous, tempS );
#endif

/* Sample rate ----------- */
#if USE_POINTER
	PRINT(("Attempt to set siSampleRate to 44100 using pointer method.\n"));
	tempF = ((unsigned long)44100) << 16;
	err = SPBSetDeviceInfo(mRefNum, siSampleRate, (Ptr) &tempF);
	if (err) PRINT(("setting siSampleRate using pointer failed\n"));
	tempF = 0;
	PrintDeviceInfo( siSampleRate, tempF );
	
	PRINT(("Attempt to set siSampleRate to 22050 using pointer method.\n"));
	tempF = ((unsigned long)22050) << 16;
	err = SPBSetDeviceInfo(mRefNum, siSampleRate, (Ptr) &tempF);
	if (err) PRINT(("setting siSampleRate using pointer failed\n"));
	tempF = 0;
	PrintDeviceInfo( siSampleRate, tempF );
#else
	PRINT(("Attempt to set siSampleRate to 44100 using direct method.\n"));
	err = SPBSetDeviceInfo(mRefNum, siSampleRate, (Ptr) (((unsigned long)44100) << 16));
	if (err) PRINT(("setting siSampleRate using direct failed\n"));
	PrintDeviceInfo( siSampleRate, tempF );
	
	PRINT(("Attempt to set siSampleRate to 22050 using direct method.\n"));
	err = SPBSetDeviceInfo(mRefNum, siSampleRate, (Ptr)  (((unsigned long)44100) << 16));
	if (err) PRINT(("setting siSampleRate using direct failed\n"));
	PrintDeviceInfo( siSampleRate, tempF );
#endif

/* All done so close up device. */
	if( mRefNum )  SPBCloseDevice(mRefNum);
	return 0;
	
error:
	if( mRefNum )  SPBCloseDevice(mRefNum);
	return -1;
}