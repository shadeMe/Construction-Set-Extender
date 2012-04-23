#ifndef simd_h
#define simd_h

/*
  Very simple utility to detect simd capability of a cpu. Written by
  Arthur Miller 5. july, 2003. You may use it or distribute it in any
  way you wish, as long as you keep this notice.
  For contact visit: www.nextpoint.se.

  Usage: include this file in your source. Call simd_init().
  Then you can test for a simd version like this:

          if( HASSSE )
	     do something here

  For avialable definitions see code below.
 */

#ifdef __cplusplus
    extern "C" {
#endif

static unsigned simd = 0x0;

#define MMX         0x00800000
#define MMXPLUS		0x00400000
#define SSE         0x02000000
#define SSE2		0x04000000
#define	SSE3		0x00000001
#define SSSE3		0x00000200
#define	SSE41		0x00080000
#define	SSE42		0x00100000
#define	SSE4A		0x00000040
#define	SSE5		0x00000800
#define A3DNOW		0x80000000
#define A3DNOWEXT	0x40000000

#define HASMMX		( simd & MMX )
#define HASSSE		( simd & SSE )
#define HASSSE2		( simd & SSE2 )
#define HASSSE3		( simd & SSE3 )
#define HASSSEE3	( simd & SSE3 )
#define HASSSE5		( simd & SSE5 )
#define HASSSE41	( simd & SSE41 )
#define HASSSE42	( simd & SSE42 )
#define HASSSE4A	( simd & SSE4A )
#define HAS3DNOW	( simd & A3DNOW )
#define HAS3DNOWEXT	( simd & A3DNOWEXT )

void simd_init(){
	int info[4];
	int infoext[4];

	unsigned ecx = 0;
	unsigned edx = 0;

	__cpuid(info, 0);

	if( info[0] >= 1 )
	{
		__cpuid(info, 1);
		ecx = info[2];
		edx = info[3];
	}

	__cpuid(info, 0x80000000);

	if( info[0] >= 0x80000001 )
		__cpuid(infoext, 0x80000001);

	if( MMX & edx ) simd |= MMX;

	if( SSE & edx ) simd |= SSE;

	if( SSE2 & edx ) simd |= SSE2;

	if( SSE3 & ecx ) simd |= SSE3;

	if( SSSE3 & ecx ) simd |= SSSE3;

	if( SSE41 & ecx ) simd |= SSE41;

	if( SSE42 & ecx ) simd |= SSE42;

	if( SSE5 & infoext[2] ) simd |= SSE5;

	if( SSE4A & infoext[2] ) simd |= SSE4A;

	if( A3DNOW & infoext[3] ) simd |= A3DNOW;

	if( MMXPLUS & infoext[3] ) simd |= MMXPLUS;

	if( A3DNOWEXT & infoext[3] ) simd |= A3DNOWEXT;
}

#ifdef __cplusplus
	}
#endif

#endif