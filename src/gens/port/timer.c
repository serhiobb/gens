#include <unistd.h>
#if HAVE_LIBRT
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stddef.h>
#include "timer.h"


// Time #defines from WINE's dlls/ntdll/time.c
#define TICKSPERSEC        10000000
#define SECSPERDAY         86400
#define SECS_1601_TO_1970  ((369 * 365 + 89) * (unsigned long long)SECSPERDAY)
#define TICKS_1601_TO_1970 (SECS_1601_TO_1970 * TICKSPERSEC)

static long long start_time;


/**
 * NtQuerySystemTime(): Retrieves the current system time.
 * @param time Pointer to 64-bit variable to store the time in.
 */
static void NtQuerySystemTime(long long* time)
{
	// Originally from Wine 1.1.6
	// dlls/ntdll/time.c
	
	if (!time)
		return;
	
#if HAVE_LIBRT
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	*time = now.tv_sec * (long long)TICKSPERSEC + TICKS_1601_TO_1970;
	*time += (now.tv_nsec / 1000) * 10;
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	*time = now.tv_sec * (long long)TICKSPERSEC + TICKS_1601_TO_1970;
	*time += now.tv_usec * 10;
#endif
}


/**
 * init_timer(): Initialize the Wine timer functions.
 */
void init_timer(void)
{
	NtQuerySystemTime(&start_time);
}


/**
 * GetTickCount() Get the number of milliseconds that have elapsed since Gens was started.
 * @return Milliseconds since Gens was started.
 */
unsigned int GetTickCount(void)
{
	// Originally from Wine 1.1.6
	// dlls/ntdll/time.c
	
	long long now;
	NtQuerySystemTime(&now);
	return (now - start_time) / 10000;
}


/**
 * QueryPerformanceFrequency(): Get the frequency of the high-resolution performance counter.
 * @param freq Pointer to 64-bit variable to store the frequency in.
 */
void QueryPerformanceFrequency(long long *frequency)
{
	// Originally from Wine 1.1.6
	// dlls/ntdll/time.c
	
	// The frequency is always reported as 1,193,182 Hz.
	// My XP Pro VM reports the frequency as 3,579,545 Hz (NTSC color subcarrier),
	// but it doesn't really make a difference.
	*frequency = 1193182;
}


/**
 * QueryPerformanceCounter(): Get the high-resolution performance counter value.
 * @param counter Pointer to 64-bit variable to store the counter in.
 */
void QueryPerformanceCounter(long long *counter)
{
	// Originally from Wine 1.1.6
	// dlls/ntdll/time.c
	
	if (!counter)
		return;
	
	long long now;
	NtQuerySystemTime(&now);
	*counter = ((now - start_time) * 21) / 176;
}
