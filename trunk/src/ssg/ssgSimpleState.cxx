
#include "ssgLocal.h"

void _ssgForceLineState ()
{
  _ssgCurrentContext->getState()->enables &= ~((1<<SSG_GL_TEXTURE_EN) |
                                (1<<SSG_GL_LIGHTING_EN)|
                                (1<<SSG_GL_COLOR_MATERIAL_EN)) ;
  glDisable ( GL_TEXTURE_2D ) ;
  glDisable ( GL_COLOR_MATERIAL ) ;
  glDisable ( GL_LIGHTING  ) ;
  glDisable ( GL_DEPTH_TEST ) ;
}


ssgSimpleState::ssgSimpleState ( int /* I_am_currstate */ )
{
  type |= SSG_TYPE_SIMPLESTATE ;
  filename  = NULL ;
  dont_care = 0xFFFFFFFF ;
  disable ( GL_TEXTURE_2D  ) ;
  enable  ( GL_CULL_FACE   ) ;
  enable  ( GL_COLOR_MATERIAL ) ;
  enable  ( GL_LIGHTING ) ;
  disable ( GL_BLEND ) ;
  enable  ( GL_ALPHA_TEST ) ;
  setShadeModel ( GL_SMOOTH ) ;
}

ssgSimpleState::ssgSimpleState (void)
{
  type |= SSG_TYPE_SIMPLESTATE ;

  filename  = NULL ;
  dont_care = 0xFFFFFFFF ;
}

ssgSimpleState::~ssgSimpleState (void)
{
  delete filename ;
}

void ssgSimpleState::apply (void)
{
  if ( ~ dont_care & ( (1<<SSG_GL_COLOR_MATERIAL) |
                       (1<<SSG_GL_DIFFUSE       ) |
                       (1<<SSG_GL_AMBIENT       ) |
                       (1<<SSG_GL_SPECULAR      ) |
                       (1<<SSG_GL_EMISSION      ) |
                       (1<<SSG_GL_SHININESS     ) ) )
  {
    int switched_modes = FALSE ;

    if ( ~ dont_care & (1<<SSG_GL_COLOR_MATERIAL ) &&
      _ssgCurrentContext->getState()->colour_material_mode != colour_material_mode )
    {
      glColorMaterial ( GL_FRONT_AND_BACK, (GLenum) colour_material_mode ) ;
      _ssgCurrentContext->getState()->colour_material_mode = colour_material_mode ;
      switched_modes = TRUE ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_SHININESS) ) &&
      _ssgCurrentContext->getState()->shininess != shininess )
    {
      glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
      _ssgCurrentContext->getState()->shininess = shininess ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_SPECULAR) ) &&
      ( switched_modes ||
        ! sgEqualVec3 ( _ssgCurrentContext->getState()->specular_colour, specular_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->specular_colour, specular_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_EMISSION) ) &&
      ( switched_modes ||
      ! sgEqualVec3 ( _ssgCurrentContext->getState()->emission_colour, emission_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->emission_colour, emission_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_AMBIENT) ) &&
      ( switched_modes ||
      ! sgEqualVec3 ( _ssgCurrentContext->getState()->ambient_colour, ambient_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->ambient_colour, ambient_colour ) ;
    }

    if ( ( ~ dont_care & (1<<SSG_GL_DIFFUSE) ) &&
      ( switched_modes ||
      ! sgEqualVec4 ( _ssgCurrentContext->getState()->diffuse_colour, diffuse_colour ) ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
      sgCopyVec4 ( _ssgCurrentContext->getState()->diffuse_colour, diffuse_colour ) ;
    }
  }

  int turn_on  = ~dont_care &  enables & ~_ssgCurrentContext->getState()->enables & 0x3F ;
  int turn_off = ~dont_care & ~enables &  _ssgCurrentContext->getState()->enables & 0x3F ;

  (*(__ssgEnableTable [turn_on ]))() ;
  (*(__ssgDisableTable[turn_off]))() ;

  _ssgCurrentContext->getState()->enables |=  turn_on  ;
  _ssgCurrentContext->getState()->enables &= ~turn_off ;

  if ( ( ~ dont_care & (1<<SSG_GL_TEXTURE) ) && 
    _ssgCurrentContext->getState()->texture_handle != texture_handle )
  {
    stats_bind_textures++ ;
#ifdef GL_VERSION_1_1
    glBindTexture ( GL_TEXTURE_2D, texture_handle ) ;
#else
    /* For ancient SGI machines */
    glBindTextureEXT ( GL_TEXTURE_2D, texture_handle ) ;
#endif
    _ssgCurrentContext->getState()->texture_handle = texture_handle ;
  }

  if ( ( ~ dont_care & (1<<SSG_GL_SHADE_MODEL) ) &&
    _ssgCurrentContext->getState()->shade_model != shade_model )
  {
    glShadeModel ( shade_model ) ;
    _ssgCurrentContext->getState()->shade_model = shade_model ;
  }

  if ( ( ~ dont_care & (1<<SSG_GL_ALPHA_TEST) ) &&
    _ssgCurrentContext->getState()->alpha_clamp != alpha_clamp )
  {
    glAlphaFunc ( GL_GREATER, alpha_clamp ) ;
    _ssgCurrentContext->getState()->alpha_clamp = alpha_clamp ;
  }
}

void ssgSimpleState::force (void)
{
/*
  glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
  _ssgCurrentContext->getState()->shininess = shininess ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
  sgCopyVec3 ( _ssgCurrentContext->getState()->specular_colour, specular_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
  sgCopyVec3 ( _ssgCurrentContext->getState()->emission_colour, emission_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
  sgCopyVec3 ( _ssgCurrentContext->getState()->ambient_colour, ambient_colour ) ;
  glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
  sgCopyVec4 ( _ssgCurrentContext->getState()->diffuse_colour, diffuse_colour ) ;
*/
  if ( ~ dont_care & ( (1<<SSG_GL_COLOR_MATERIAL ) |
                       (1<<SSG_GL_DIFFUSE        ) |
                       (1<<SSG_GL_AMBIENT        ) |
                       (1<<SSG_GL_SPECULAR       ) |
                       (1<<SSG_GL_EMISSION       ) |
                       (1<<SSG_GL_SHININESS      ) ) )
  {
    if ( ~ dont_care & (1<<SSG_GL_COLOR_MATERIAL ) )
    {
      glColorMaterial ( GL_FRONT_AND_BACK, (GLenum) colour_material_mode ) ;
      _ssgCurrentContext->getState()->colour_material_mode = colour_material_mode ;
    }

    if ( ~ dont_care & (1<<SSG_GL_SHININESS ) )
    {
      glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess ) ;
      _ssgCurrentContext->getState()->shininess = shininess ;
    }

    if ( ~ dont_care & (1<<SSG_GL_DIFFUSE ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_colour ) ;
      sgCopyVec4 ( _ssgCurrentContext->getState()->diffuse_colour, diffuse_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_EMISSION ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, emission_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->emission_colour, emission_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_AMBIENT ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT, ambient_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->ambient_colour, ambient_colour ) ;
    }

    if ( ~ dont_care & (1<<SSG_GL_SPECULAR ) )
    {
      glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_colour ) ;
      sgCopyVec3 ( _ssgCurrentContext->getState()->specular_colour, specular_colour ) ;
    }
  }

  int turn_on  = ~dont_care &  enables & 0x3F ;
  int turn_off = ~dont_care & ~enables & 0x3F ;

  (*(__ssgEnableTable [turn_on ]))() ;
  (*(__ssgDisableTable[turn_off]))() ;

  _ssgCurrentContext->getState()->enables |=  turn_on  ;
  _ssgCurrentContext->getState()->enables &= ~turn_off ;

  if ( ~ dont_care & (1<<SSG_GL_TEXTURE ) )
  {
    stats_bind_textures++ ;
#ifdef GL_VERSION_1_1
    glBindTexture ( GL_TEXTURE_2D, texture_handle ) ;
#else
    /* For ancient SGI machines */
    glBindTextureEXT ( GL_TEXTURE_2D, texture_handle ) ;
#endif
    _ssgCurrentContext->getState()->texture_handle = texture_handle ;
  }

  if ( ~ dont_care & (1<<SSG_GL_SHADE_MODEL ) )
  {
    glShadeModel ( shade_model ) ;
    _ssgCurrentContext->getState()->shade_model = shade_model ;
  }

  if ( ~ dont_care & (1<<SSG_GL_ALPHA_TEST ) )
  {
    glAlphaFunc ( GL_GREATER, alpha_clamp ) ;
    _ssgCurrentContext->getState()->alpha_clamp = alpha_clamp ;
  }
}

void ssgSimpleState::print ( FILE *fd, char *indent )
{
  ssgState::print ( fd, indent ) ;
}



int ssgSimpleState::isEnabled ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      return enables & (1<<SSG_GL_TEXTURE_EN) ;

    case GL_CULL_FACE      :
      return enables & (1<<SSG_GL_CULL_FACE_EN) ;

    case GL_COLOR_MATERIAL :
      return enables & (1<<SSG_GL_COLOR_MATERIAL_EN) ;

    case GL_LIGHTING :
      return enables & (1<<SSG_GL_LIGHTING_EN) ;

    case GL_BLEND          :
      return enables & (1<<SSG_GL_BLEND_EN) ;

    case GL_ALPHA_TEST     :
      return enables & (1<<SSG_GL_ALPHA_TEST_EN) ;

    default :
      return FALSE ;
  }
}



void ssgSimpleState::disable ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      enables &= ~ (1<<SSG_GL_TEXTURE_EN) ;
      care_about ( SSG_GL_TEXTURE_EN ) ;
      break ;

    case GL_CULL_FACE      :
      enables &= ~ (1<<SSG_GL_CULL_FACE_EN) ;
      care_about ( SSG_GL_CULL_FACE_EN ) ;
      break ;

    case GL_COLOR_MATERIAL :
      enables &= ~ (1<<SSG_GL_COLOR_MATERIAL_EN) ;
      care_about ( SSG_GL_COLOR_MATERIAL_EN ) ;
      break ;

    case GL_LIGHTING :
      enables &= ~ (1<<SSG_GL_LIGHTING_EN) ;
      care_about ( SSG_GL_LIGHTING_EN ) ;
      break ;

    case GL_BLEND          :
      enables &= ~ (1<<SSG_GL_BLEND_EN) ;
      care_about ( SSG_GL_BLEND_EN ) ;
      break ;

    case GL_ALPHA_TEST     :
      enables &= ~ (1<<SSG_GL_ALPHA_TEST_EN) ;
      care_about ( SSG_GL_ALPHA_TEST_EN ) ;
      break ;

    default :
      fprintf ( stderr,
	     "Illegal mode passed to ssgSimpleState::disable(%d)\n",
			       mode ) ;
      break ; 
  }
}

void ssgSimpleState::enable  ( GLenum mode )
{
  switch ( mode )
  {
    case GL_TEXTURE_2D        :
      enables |=  (1<<SSG_GL_TEXTURE_EN) ;
      care_about ( SSG_GL_TEXTURE_EN ) ;
      break ;

    case GL_CULL_FACE      :
      enables |=  (1<<SSG_GL_CULL_FACE_EN) ;
      care_about ( SSG_GL_CULL_FACE_EN ) ;
      break ;

    case GL_COLOR_MATERIAL :
      enables |=  (1<<SSG_GL_COLOR_MATERIAL_EN) ;
      care_about ( SSG_GL_COLOR_MATERIAL_EN ) ;
      break ;

    case GL_BLEND          :
      enables |=  (1<<SSG_GL_BLEND_EN) ;
      care_about ( SSG_GL_BLEND_EN ) ;
      break ;

    case GL_ALPHA_TEST     :
      enables |=  (1<<SSG_GL_ALPHA_TEST_EN) ;
      care_about ( SSG_GL_ALPHA_TEST_EN ) ;
      break ;

    case GL_LIGHTING :
      enables |= (1<<SSG_GL_LIGHTING_EN) ;
      care_about ( SSG_GL_LIGHTING_EN ) ;
      break ;

    default :
      fprintf ( stderr,
	     "Illegal mode passed to ssgSimpleState::enable(%d)\n",
			       mode ) ;
      break ; 
  }
}


int ssgSimpleState::load ( FILE *fd )
{
  delete filename ;

  _ssgReadInt   ( fd, & dont_care            ) ;
  _ssgReadInt   ( fd, & enables              ) ;
  _ssgReadString( fd, & filename             ) ;
  _ssgReadInt   ( fd, & wrapu                ) ;
  _ssgReadInt   ( fd, & wrapv                ) ;
  _ssgReadInt   ( fd, & colour_material_mode ) ;
  _ssgReadVec4  ( fd, specular_colour        ) ;
  _ssgReadVec4  ( fd, emission_colour        ) ;
  _ssgReadVec4  ( fd, ambient_colour         ) ;
  _ssgReadVec4  ( fd, diffuse_colour         ) ;
  _ssgReadInt   ( fd, (int *)(& shade_model) ) ;
  _ssgReadFloat ( fd, & shininess            ) ;
  _ssgReadFloat ( fd, & alpha_clamp          ) ;

  if ( filename != NULL && filename[0] != '\0' )
    setTexture ( filename, wrapu, wrapv ) ;
  else
    texture_handle = 0 ;

  return ssgState::load(fd) ;
}


int ssgSimpleState::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, dont_care            ) ;
  _ssgWriteInt   ( fd, enables              ) ;
  _ssgWriteString( fd, filename             ) ;
  _ssgWriteInt   ( fd, wrapu                ) ;
  _ssgWriteInt   ( fd, wrapv                ) ;
  _ssgWriteInt   ( fd, colour_material_mode ) ;
  _ssgWriteVec4  ( fd, specular_colour      ) ;
  _ssgWriteVec4  ( fd, emission_colour      ) ;
  _ssgWriteVec4  ( fd, ambient_colour       ) ;
  _ssgWriteVec4  ( fd, diffuse_colour       ) ;
  _ssgWriteInt   ( fd, (int) shade_model    ) ;
  _ssgWriteFloat ( fd, shininess            ) ;
  _ssgWriteFloat ( fd, alpha_clamp          ) ;

  return ssgState::save(fd) ;
}

void ssgSimpleState::setTextureFilename ( char *fname )
{
  delete filename ;

  if ( fname == NULL )
    filename = NULL ;
  else
  {
    filename = new char [ strlen(fname)+1 ] ;
    strcpy ( filename, fname ) ;
  }
}

void ssgSimpleState::setTexture ( char *fname, int _wrapu, int _wrapv )
{
  wrapu = _wrapu ; wrapv = _wrapv ;
  ssgTexture *tex = new ssgTexture ( fname, wrapu, wrapv ) ;
  setTexture ( tex ) ;
  delete tex ;
}


