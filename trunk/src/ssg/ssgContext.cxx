
#include "ssgLocal.h"

ssgContext *_ssgCurrentContext = NULL ;

ssgContext::~ssgContext ()
{
  if ( isCurrent() )
    _ssgCurrentContext = NULL ;

  delete currentState ;
  delete basicState ;
  delete frustum ;
}


ssgContext::ssgContext ()
{
  makeCurrent () ;
  currentState     = NULL  ;
  basicState       = NULL  ;
  orthographic     = FALSE ;
  sgMakeIdentMat4 ( cameraMatrix ) ;
  cullFace         = TRUE  ;
  ovTexture        = FALSE ;
  ovCullface       = FALSE ;

  frustum = new sgFrustum ;
  frustum -> setNearFar ( 1.0, 10000.0 ) ;
  frustum -> setFOV     ( 60.0, 45.0 ) ;

  glEnable ( GL_CULL_FACE ) ;

  currentState = new ssgSimpleState ( 1 ) ;
  basicState   = new ssgSimpleState ( 0 ) ;

  currentState->force () ;

  basicState->dont_care      = 0 ;
  basicState->texture_handle = 0 ;
  basicState->colour_material_mode = GL_AMBIENT_AND_DIFFUSE ;
  sgSetVec4(basicState->specular_colour,1.0f,1.0f,1.0f,1.0f);
  sgSetVec4(basicState->emission_colour,0.0f,0.0f,0.0f,1.0f);
  sgSetVec4(basicState->ambient_colour, 1.0f,1.0f,1.0f,1.0f);
  sgSetVec4(basicState->diffuse_colour, 1.0f,1.0f,1.0f,1.0f);
  basicState->shade_model = GL_SMOOTH ;
  basicState->shininess   = 0.0f ;
  basicState->alpha_clamp = 0.01f ;
}


int ssgContext::isCurrent ()
{
  return _ssgCurrentContext == this ;
}

void ssgContext::makeCurrent ()
{
  _ssgCurrentContext = this ;
}

void ssgContext::getCameraPosition ( sgVec3 pos )
{
  sgCopyVec3 ( pos, cameraMatrix [ 3 ] ) ;
}

void ssgContext::overrideTexture ( int on_off )
{
  ovTexture = on_off ;
}

void ssgContext::overrideCullface ( int on_off )
{
  ovCullface = on_off ;
}

void ssgContext::getNearFar ( float *n, float *f )
{
  frustum -> getNearFar ( n, f ) ;
}

void ssgContext::getFOV ( float *w, float *h )
{
  frustum -> getFOV ( w, h ) ;
}

void ssgContext::setNearFar ( float n, float f )
{
  frustum -> setNearFar ( n, f ) ;
}

void ssgContext::getOrtho ( float *w, float *h )
{
  if ( w != NULL )
    *w = frustum -> getRight() - frustum -> getLeft() ;

  if ( h != NULL )
    *h = frustum -> getTop() - frustum -> getBot() ;
}

void ssgContext::setOrtho ( float w, float h )
{
  orthographic = TRUE ;
  frustum -> setFrustum ( -w/2, w/2, -h/2, h/2,
                          frustum -> getNear (),
                          frustum -> getFar () ) ;
}

void ssgContext::setFOV ( float w, float h )
{
  orthographic = FALSE ;
  frustum -> setFOV ( w, h ) ;
}

void ssgContext::setCamera ( sgMat4 mat )
{
  sgMat4 viewmat ;

  sgTransposeNegateMat4 ( viewmat, mat ) ;

  sgCopyMat4    ( cameraMatrix, _ssgOpenGLAxisSwapMatrix ) ;
  sgPreMultMat4 ( cameraMatrix, viewmat ) ;
}

void ssgContext::setCamera ( sgCoord *coord )
{
  sgMat4 viewmat, mat ;

  sgMakeCoordMat4 ( mat, coord ) ;
  sgTransposeNegateMat4 ( viewmat, mat ) ;

  sgCopyMat4      ( cameraMatrix, _ssgOpenGLAxisSwapMatrix ) ;
  sgPreMultMat4   ( cameraMatrix, viewmat ) ;
}

void ssgContext::loadProjectionMatrix()
{
  glLoadIdentity () ;
  pushProjectionMatrix () ;
}

void ssgContext::pushProjectionMatrix ()
{
  pushProjectionMatrix ( frustum ) ;
}

void ssgContext::pushProjectionMatrix ( sgFrustum *f )
{
  if ( orthographic )
    glOrtho   ( f -> getLeft() , f -> getRight(),
                f -> getBot () , f -> getTop  (),
                f -> getNear() , f -> getFar  () ) ;
  else
    glFrustum ( f -> getLeft() , f -> getRight(),
                f -> getBot () , f -> getTop  (),
                f -> getNear() , f -> getFar  () ) ;
}

void ssgContext::getProjectionMatrix ( sgMat4 dst )
{
  if ( orthographic )
  {
    float l =  frustum -> getLeft  () ;
    float r =  frustum -> getRight () ;
    float b =  frustum -> getBot   () ;
    float t =  frustum -> getTop   () ;
    float n =  frustum -> getNear  () ;
    float f =  frustum -> getFar   () ;

    sgMakeIdentMat4 ( dst ) ;
    dst[0][0] =  2.0f / ( r - l ) ;
    dst[1][1] =  2.0f / ( t - b ) ;
    dst[2][2] = -2.0f / ( f - n ) ;

    dst[3][0] = - ( r + l ) / ( r - l ) ;
    dst[3][1] = - ( t + b ) / ( t - b ) ;
    dst[3][2] = - ( f + n ) / ( f - n ) ;
  }
  else
    frustum -> getMat4 ( dst ) ;
}

void ssgContext::getModelviewMatrix ( sgMat4 dst )
{
  sgCopyMat4 ( dst, cameraMatrix ) ;
}

void ssgContext::loadModelviewMatrix ()
{
  glLoadMatrixf ( (float *) cameraMatrix ) ;
}

void ssgContext::loadModelviewMatrix ( sgMat4 mat )
{
  glLoadMatrixf ( (float *) mat ) ;
}

void ssgContext::cull ( ssgRoot *r )
{
  r -> cull ( frustum, cameraMatrix, TRUE ) ;
}

