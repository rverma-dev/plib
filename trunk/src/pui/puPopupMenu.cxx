/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

#include "puLocal.h"

puObject *puPopupMenu::add_item ( const char *str, puCallback _cb )
{
  int w, h ;
  getSize ( &w, &h ) ;
  puOneShot *b = new puOneShot ( 0, h, str ) ;
  b->setStyle        ( PUSTYLE_PLAIN ) ;
  b->setColourScheme ( colour[PUCOL_FOREGROUND][0],
		       colour[PUCOL_FOREGROUND][1],
		       colour[PUCOL_FOREGROUND][2],
		       colour[PUCOL_FOREGROUND][3] ) ;
  b->setCallback     ( _cb ) ;
  b->setLegendPlace  ( PUPLACE_LOWER_LEFT ) ;
  recalc_bbox () ;
  return b ;
}

void puPopupMenu::close ( void )
{
  puPopup::close () ;

  int widest = 0 ;
  puObject *ob = dlist ;

  /*
   * June 17th, 1998, Shammi
   * There seems to be some mismatch with the
   * #define pumenusize and the actual size
   * There seems to be some overlap resulting
   * in more than one option being highlighted.
   * By setting the size to the actual values,
   * the overlap area seems to be less now.
   */

  int w, h ;

  for ( ob = dlist ; ob != NULL ; ob = ob -> getNextObject() )
  {
    ob -> getSize ( &w, &h ) ;

    if ( w > widest ) widest = w ;
  }

  for ( ob = dlist ; ob != NULL ; ob = ob -> getNextObject() )
  {
    ob -> getSize ( &w, &h ) ;
    ob -> setSize ( widest, h ) ;
  }

  recalc_bbox () ;
}


int puPopupMenu::checkKey ( int key, int updown )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown == PU_DOWN )
  {
    hide () ;

    /* Turn everything off ready for next time. */

    for ( puObject *bo = dlist ; bo != NULL ; bo = bo -> getNextObject() )
      bo -> clrValue () ;
  }

  puObject *bo ;

  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  for ( bo = dlist ;
        bo -> getNextObject() != NULL ;
        bo = bo -> getNextObject() )
    /* Find the last object in our list. */ ;

  for ( ; bo != NULL ; bo = bo -> getPrevObject() )
    if ( bo -> checkKey ( key, updown ) )
      return TRUE ;

  return FALSE ;
}


int puPopupMenu::checkHit ( int button, int updown, int x, int y )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () || ( window != puGetWindow () ) )
    return FALSE ;

  /* Must test 'isHit' before making the menu invisible! */

  int hit = isHit ( x, y ) ;

  /*
   * June 17th, 1998, Shammi :
   * There seemed to be a miscalculation with the menus initially
   * Therefore I moved the recalculation stuff before the clearing.
   */

  /*
    This might be a bit redundant - but it's too hard to keep
    track of changing abox sizes when daughter objects are
    changing sizes.
  */

  recalc_bbox();
  x -= abox.min[0] ;
  y -= abox.min[1] ;

  /*
   * June 17th, 1998, Shammi :
   * Also clear the menu when the dragging the mouse and not hit.
   */

  if (   updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN ||
       ( updown == PU_DRAG && !hit ) )
  {

    /* June 17th, 1998, Shammi :
     * Do not hide the menu if mouse is dragged out
     */

    if ( updown != PU_DRAG )
      hide () ;

    /* Turn everything off ready for next time. */

    /* June 17th, 1998, Shammi:
     * Make sure we check for a hit, if the mouse is moved
     * out of the menu.
     */

    for ( puObject *bo = dlist ; bo != NULL ; bo = bo -> getNextObject() )
    {
      if ( ! hit )
        bo -> checkHit ( button, updown, x , y ) ;

      bo -> clrValue () ;
    }
  }

  if ( ! hit )
    return FALSE ;

  puObject *bo ;
  
  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  /* June 17th, 1998, Shammi :
   * If the mouse is dragged and the menuItem is not hit, 
   * clear it
   */

  for ( bo = dlist ;
        bo -> getNextObject() != NULL ;
        bo = bo -> getNextObject() )
    if ( updown == PU_DRAG && ! bo -> checkHit ( button, updown, x, y ) )
      bo -> clrValue () ;

    /* Find the last object in our list. */ ;

  for ( ; bo != NULL ; bo = bo -> getPrevObject() )
    if ( bo -> checkHit ( button, updown, x, y ) )
      return TRUE ;

  return FALSE ;
}


