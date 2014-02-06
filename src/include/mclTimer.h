#ifndef MCL_TIMER_H
#define MCL_TIMER_H

/** \file mclTimer.h
 * \brief A real-time timer.
 */

#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif

#include <time.h>

#define usec_per_sec 1000000

//! class of timer objects.
class mclTimer {
 public:
  //! constructs a timer that has already expired
  mclTimer();
  /** constructs a timer that will expire at the specified time from now
   * the expiration time on the constructed timer will be seconds+useconds
   * from the current time.
   * \param seconds expiration time from now, in seconds
   * \param useconds expiration time from now, in microseconds
   */
  mclTimer(int seconds, int useconds);

  static double getTimeDouble();

  //! true if the timer has expired.
  bool expired();

  /** resets the timer so that it will expire at the specified time from now
   * the expiration time on the timer will be seconds+useconds
   * from the current time.
   * \param seconds expiration time from now, in seconds
   * \param useconds expiration time from now, in microseconds
   */
  void restart(int seconds, int useconds);

  /** resets the timer so that it will expire in 1/Hz seconds
   * the expiration time will allow the timer to fire at the specified 
   * frequency if it is continuously reset.
   * \param Hz the timer frequency in Hertz
   */
  void restartHz(float Hz);

  /** computes the time to expiration in microseconds and seconds
   * \param sleft writes the number of seconds left to this memory location
   * \param usleft writes the number of microseconds left to this memory location
   * \return true if the timer is not expired
   */
  bool timeRemaining(int *sleft,int *usleft);

 private:
  struct timeval current,target;
};

#endif
