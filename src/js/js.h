#ifndef __INCLUDED_JS_H__
#define __INCLUDED_JS_H__ 1

#include <stdio.h>
#include <stdlib.h>

/*
  FreeBSD port - courtesy of Stephen Montgomery-Smith
  <stephen@math.missouri.edu>

  NetBSD mods - courtesy Rene Hexel.

  The next lines are to define BSD
  see http://www.freebsd.org/handbook/porting.html for why we do this
*/

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#ifdef WIN32
#  include <windows.h>
#  if defined( __CYGWIN32__ ) || defined( __CYGWIN__ )
#    define NEAR /* */
#    define FAR  /* */
#  endif
#  include <mmsystem.h>
#  include <string.h>
#else

#  include <unistd.h>
#  include <fcntl.h>

#  if defined(__FreeBSD__) || defined(__NetBSD__)
#    include <machine/joystick.h>
#    define JS_DATA_TYPE joystick
#    define JS_RETURN (sizeof(struct JS_DATA_TYPE))
#  elif defined(__linux__)
#    include <sys/ioctl.h>
#    include <linux/joystick.h>
#    include <errno.h>

     /* check the joystick driver version */

#    ifdef JS_VERSION
#      if JS_VERSION >= 0x010000
#        define JS_NEW
#      endif
#    endif

#  else
#    ifndef JS_DATA_TYPE

      /*
        Not Windoze and no joystick driver...

        Well - we'll put these values in and that should
        allow the code to at least compile. The JS open
        routine should error out and shut off all the code
        downstream anyway
      */

       struct JS_DATA_TYPE
       {
         int buttons ;
         int x ;
         int y ;
       } ;

#      define JS_RETURN (sizeof(struct JS_DATA_TYPE))
#    endif
#  endif
#endif

#define JS_TRUE  1
#define JS_FALSE 0

#ifdef WIN32
#  define _JS_MAX_AXES 6
#else
#  if defined(__FreeBSD__) || defined(__NetBSD__)
#  define _JS_MAX_AXES 2
#  else
#  define _JS_MAX_AXES 9
#  endif
#endif

class jsJoystick
{
#if defined(__FreeBSD__) || defined(__NetBSD__)
  int          id ;
#endif
#ifdef WIN32
  JOYINFOEX    js       ;
  UINT         js_id    ;
#else
# ifdef JS_NEW
  js_event     js          ;
  int          tmp_buttons ;
  float        tmp_axes [ _JS_MAX_AXES ] ;
# else
  JS_DATA_TYPE js ;
# endif
  char         fname [ 128 ] ;
  int          fd       ;
#endif

  int          error    ;
  int          num_axes ;
  int          num_buttons ;

  float dead_band [ _JS_MAX_AXES ] ;
  float saturate  [ _JS_MAX_AXES ] ;
  float center    [ _JS_MAX_AXES ] ;
  float max       [ _JS_MAX_AXES ] ;
  float min       [ _JS_MAX_AXES ] ;

  void open ()
  {
#ifdef WIN32
    JOYCAPS jsCaps ;

    js . dwFlags = JOY_RETURNALL ;
    js . dwSize  = sizeof ( js ) ;

    memset ( &jsCaps, 0, sizeof(jsCaps) ) ;

    error = ( joyGetDevCaps( js_id, &jsCaps, sizeof(jsCaps) )
                 != JOYERR_NOERROR ) ;

    if ( jsCaps.wNumAxes == 0 )
    {
      num_axes = 0 ;
      setError () ;
    }
    else
    {
      /* accept all the axis */
      num_axes = _JS_MAX_AXES ;
      min[5] = (float)jsCaps.wVmin ; max[5] = (float)jsCaps.wVmax ;
      min[4] = (float)jsCaps.wUmin ; max[4] = (float)jsCaps.wUmax ;
      min[3] = (float)jsCaps.wRmin ; max[3] = (float)jsCaps.wRmax ;
      min[2] = (float)jsCaps.wZmin ; max[2] = (float)jsCaps.wZmax ;
      min[1] = (float)jsCaps.wYmin ; max[1] = (float)jsCaps.wYmax ;
      min[0] = (float)jsCaps.wXmin ; max[0] = (float)jsCaps.wXmax ;
    }

    for ( int i = 0 ; i < num_axes ; i++ )
    {
      center    [ i ] = ( max[i] + min[i] ) / 2.0f ;
      dead_band [ i ] = 0.0f ;
      saturate  [ i ] = 1.0f ;
    }

#else
#  if defined(__FreeBSD__) || defined(__NetBSD__)
    FILE *joyfile;
    char joyfname[1024];
    int noargs, in_no_axes;
#  endif

    /* Default for older Linux systems. */

    num_axes    =  2 ;
    num_buttons = 32 ;

#  ifdef JS_NEW
    for ( int i = 0 ; i < _JS_MAX_AXES ; i++ )
      tmp_axes [ i ] = 0.0f ;

    tmp_buttons = 0 ;
#  endif

    fd = ::open ( fname, O_RDONLY | O_NONBLOCK ) ;

    error = ( fd < 0 ) ;

    if ( error )
      return ;

#  if defined(__FreeBSD__) || defined(__NetBSD__)

    float axes[_JS_MAX_AXES];
    int buttons[_JS_MAX_AXES];
    rawRead ( buttons, axes );
    error = axes[0] < -1000000000.0f;
    if ( error )
      return ;

    sprintf(joyfname,"%s/.joy%drc",::getenv( "HOME" ),id);

    joyfile = fopen(joyfname,"r");
    error = joyfile == NULL;
    if ( error )
      return ;
    noargs = fscanf(joyfile,"%d%f%f%f%f%f%f",&in_no_axes,
                        &min[0],&center[0],&max[0],
                        &min[1],&center[1],&max[1]);
    error = noargs != 7 || in_no_axes != _JS_MAX_AXES;
    fclose(joyfile);
    if ( error )
      return ;

    for ( int i = 0 ; i < _JS_MAX_AXES ; i++ )
    {
      dead_band [ i ] = 0.0f ;
      saturate  [ i ] = 1.0f ;
    }

#  else

    /*
      Set the correct number of axes for the linux driver
    */

#  ifdef JS_NEW
    ioctl ( fd, JSIOCGAXES   , & num_axes    ) ;
    ioctl ( fd, JSIOCGBUTTONS, & num_buttons ) ;
    fcntl ( fd, F_SETFL, O_NONBLOCK ) ;

    if ( num_axes > _JS_MAX_AXES )
      num_axes = _JS_MAX_AXES ;
#   endif

    /*
      The Linux driver seems to return 512 for all axes
      when no stick is present - but there is a chance
      that could happen by accident - so it's gotta happen
      on both axes for at least 100 attempts.
    */

#ifndef JS_NEW
    int counter = 0 ;

    do
    { 
      rawRead ( NULL, center ) ;
      counter++ ;
    } while ( ! error &&
                counter < 100 &&
                center[0] == 512.0f &&
                center[1] == 512.0f ) ;
   
    if ( counter >= 100 )
      setError() ;
#endif

    for ( int i = 0 ; i < _JS_MAX_AXES ; i++ )
    {
#ifdef JS_NEW
      max [ i ] = 32767.0f ;
      center [ i ] = 0.0f ;
      min [ i ] = -32767.0f ;
#else
      max [ i ] = center [ i ] * 2.0f ;
      min [ i ] = 0.0f ;
#endif
      dead_band [ i ] = 0.0f ;
      saturate  [ i ] = 1.0f ;
    }

#  endif
#endif
  }

  void close ()
  {
#ifndef WIN32
    if ( ! error )
      ::close ( fd ) ;
#endif
  }

  float fudge_axis ( float value, int axis )
  {
    if ( value < center[axis] )
    {
      float xx = (      value    - center[ axis ] ) /
		 ( center [ axis ] - min [ axis ] ) ;

      if ( xx < -saturate [ axis ] )
	return -1.0f ;

      if ( xx > -dead_band [ axis ] )
	return 0.0f ;

      xx = (        xx         + dead_band [ axis ] ) /
           ( saturate [ axis ] - dead_band [ axis ] ) ;

      return ( xx < -1.0f ) ? -1.0f : xx ;
    }
    else
    {
      float xx = (     value    - center [ axis ] ) /
		 ( max [ axis ] - center [ axis ] ) ;

      if ( xx > saturate [ axis ] )
	return 1.0f ;

      if ( xx < dead_band [ axis ] )
	return 0.0f ;

      xx = (        xx         - dead_band [ axis ] ) /
           ( saturate [ axis ] - dead_band [ axis ] ) ;

      return ( xx > 1.0f ) ? 1.0f : xx ;
    }
  }

public:

  jsJoystick ( int ident = 0 )
  {
#ifdef WIN32
    switch ( ident )
    {
      case 0  : js_id = JOYSTICKID1 ; open () ; break ;
      case 1  : js_id = JOYSTICKID2 ; open () ; break;
      default :    num_axes = 0 ; setError () ; break ;
    }
#else
#  if defined(__FreeBSD__) || defined(__NetBSD__)
    id = ident;
    sprintf ( fname, "/dev/joy%d", ident ) ;
#  else
    sprintf ( fname, "/dev/js%d", ident ) ;
#  endif
    open () ;
#endif
  }

  ~jsJoystick ()
  {
    close () ;
  }

  int  getNumAxes () { return num_axes ; }
  int  notWorking () { return error ;    }
  void setError   () { error = JS_TRUE ; }

  float getDeadBand ( int axis )             { return dead_band [ axis ] ; }
  void  setDeadBand ( int axis, float db )   { dead_band [ axis ] = db   ; }

  float getSaturation ( int axis )           { return saturate [ axis ]  ; }
  void  setSaturation ( int axis, float st ) { saturate [ axis ] = st    ; }

  void setMinRange ( float *axes ) { memcpy ( min   , axes, num_axes * sizeof(float) ) ; }
  void setMaxRange ( float *axes ) { memcpy ( max   , axes, num_axes * sizeof(float) ) ; }
  void setCenter   ( float *axes ) { memcpy ( center, axes, num_axes * sizeof(float) ) ; }

  void getMinRange ( float *axes ) { memcpy ( axes, min   , num_axes * sizeof(float) ) ; }
  void getMaxRange ( float *axes ) { memcpy ( axes, max   , num_axes * sizeof(float) ) ; }
  void getCenter   ( float *axes ) { memcpy ( axes, center, num_axes * sizeof(float) ) ; }

  void read ( int *buttons, float *axes )
  {
    if ( error )
    {
      if ( buttons )
        *buttons = 0 ;

      if ( axes )
        for ( int i = 0 ; i < num_axes ; i++ )
          axes[i] = 0.0f ;
    }

    float raw_axes [ _JS_MAX_AXES ] ;

    rawRead ( buttons, raw_axes ) ;

    if ( axes )
      for ( int i = 0 ; i < num_axes ; i++ )
        axes[i] = fudge_axis ( raw_axes[i], i ) ; 
  }

  void rawRead ( int *buttons, float *axes )
  {
    if ( error )
    {
      if ( buttons )
        *buttons = 0 ;

      if ( axes )
        for ( int i = 0 ; i < num_axes ; i++ )
          axes[i] = 1500.0f ;

      return ;
    }

#ifdef WIN32
    MMRESULT status = joyGetPosEx ( js_id, &js ) ;

    if ( status != JOYERR_NOERROR )
    {
      setError() ;
      return ;
    }

    if ( buttons )
      *buttons = (int) js . dwButtons ;

    if ( axes )
    {
      /* WARNING - Fall through case clauses!! */

      switch ( num_axes )
      {
	case 6: axes[5] = (float) js . dwVpos ;
	case 5: axes[4] = (float) js . dwUpos ;
	case 4: axes[3] = (float) js . dwRpos ;
	case 3: axes[2] = (float) js . dwZpos ;
	case 2: axes[1] = (float) js . dwYpos ;
	case 1: axes[0] = (float) js . dwXpos ;
      }
    }
#else

# ifdef JS_NEW

    while (1)
    {
      int status = ::read ( fd, &js, sizeof(js_event) ) ;

      if ( status != sizeof(js_event) )
      {
        /* use the old values */

	if ( buttons ) *buttons = tmp_buttons ;
	if ( axes    ) memcpy ( axes, tmp_axes, sizeof(float) * num_axes ) ;

	if ( errno == EAGAIN )
	  return ;

	perror( fname ) ;
	setError () ;
	return ;
      }

      switch ( js.type & ~JS_EVENT_INIT )
      {
	case JS_EVENT_BUTTON :
	  if ( js.value == 0 ) /* clear the flag */
	    tmp_buttons &= ~(1 << js.number) ;
	  else
	    tmp_buttons |=  (1 << js.number) ;
	  break ;

	case JS_EVENT_AXIS:
          if ( js.number < num_axes )
          {
	    tmp_axes [ js.number ] = (float) js.value ;

	    if ( axes )
	      memcpy ( axes, tmp_axes, sizeof(float) * num_axes ) ;
          }
	  break ;

	default:
          fprintf ( stderr, "PLIB_JS: Unrecognised /dev/js return!?!\n" ) ;

	  /* use the old values */

	  if ( buttons ) *buttons = tmp_buttons ;
	  if ( axes    ) memcpy ( axes, tmp_axes, sizeof(float) * num_axes ) ;

          return ;
      }

      if ( buttons )
	*buttons = tmp_buttons ;
    }

# else

    int status = ::read ( fd, &js, JS_RETURN ) ;

    if ( status != JS_RETURN )
    {
      perror ( fname ) ;
      setError () ;
      return ;
    }

    if ( buttons )
#  if defined(__FreeBSD__) || defined(__NetBSD__)
      *buttons = ( js.b1 ? 1 : 0 ) | ( js.b2 ? 2 : 0 ) ;
#  else
      *buttons = js.buttons ;
#  endif

    if ( axes )
    {
      axes[0] = (float) js.x ;
      axes[1] = (float) js.y ;
    }
# endif
#endif
  }
} ;

#endif

