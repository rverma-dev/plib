
#include "ssgLocal.h"

#include "ssgMSFSPalette.h"

/*
  Original source for BMP loader kindly
  donated by "Sean L. Palmer" <spalmer@pobox.com>
*/


struct BMPHeader
{
  unsigned short  FileType      ;
  unsigned int    FileSize      ;
  unsigned short  Reserved1     ;
  unsigned short  Reserved2     ;
  unsigned int    OffBits       ;
  unsigned int    Size          ;
  unsigned int    Width         ;
  unsigned int    Height        ;
  unsigned short  Planes        ;
  unsigned short  BitCount      ;
  unsigned int    Compression   ;
  unsigned int    SizeImage     ;
  unsigned int    XPelsPerMeter ;
  unsigned int    YPelsPerMeter ;
  unsigned int    ClrUsed       ;
  unsigned int    ClrImportant  ;
} ;


struct RGBA
{
  unsigned char r,g,b,a ;
} ;


void ssgLoadBMP ( const char *fname )
{
  int w, h, bpp ;
  RGBA pal [ 256 ] ;

  BMPHeader bmphdr ;

  ssgTextureManager* tm = ssgTextureManager::get () ;

  /* Open file & get size */
  FILE* fp ;
  if ( ( fp = tm -> openFile ( fname, "rb" ) ) == NULL )
  {
    perror ( "ssgLoadTexture" ) ;
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to open '%s' for reading.", tm -> getPath () ) ;
    return ;
  }

  /*
    Load the BMP piecemeal to avoid struct packing issues
  */

  tm -> setSwap ( FALSE ) ;
  bmphdr.FileType = tm -> readShort () ;

  if ( bmphdr.FileType == ((int)'B' + ((int)'M'<<8)) )
    tm -> setSwap ( FALSE ) ;
  else
  if ( bmphdr.FileType == ((int)'M' + ((int)'B'<<8)) )
    tm -> setSwap ( TRUE ) ;
  else
  {
    ulSetError ( UL_WARNING, "%s: Unrecognised magic number 0x%04x",
                            tm -> getPath (), bmphdr.FileType ) ;
    return ;
  }

  bmphdr.FileSize      = tm -> readInt   () ;
  bmphdr.Reserved1     = tm -> readShort () ;
  bmphdr.Reserved2     = tm -> readShort () ;
  bmphdr.OffBits       = tm -> readInt   () ;
  bmphdr.Size          = tm -> readInt   () ;
  bmphdr.Width         = tm -> readInt   () ;
  bmphdr.Height        = tm -> readInt   () ;
  bmphdr.Planes        = tm -> readShort () ;
  bmphdr.BitCount      = tm -> readShort () ;
  bmphdr.Compression   = tm -> readInt   () ;
  bmphdr.SizeImage     = tm -> readInt   () ;
  bmphdr.XPelsPerMeter = tm -> readInt   () ;
  bmphdr.YPelsPerMeter = tm -> readInt   () ;
  bmphdr.ClrUsed       = tm -> readInt   () ;
  bmphdr.ClrImportant  = tm -> readInt   () ;
 
  w   = bmphdr.Width  ;
  h   = bmphdr.Height ;
  bpp = bmphdr.Planes * bmphdr.BitCount ;

#ifdef PRINT_BMP_HEADER_DEBUG
  ulSetError ( UL_DEBUG, "Filetype %04x",      bmphdr.FileType      ) ;
  ulSetError ( UL_DEBUG, "Filesize %08x",      bmphdr.FileSize      ) ;
  ulSetError ( UL_DEBUG, "R1 %04x",            bmphdr.Reserved1     ) ;
  ulSetError ( UL_DEBUG, "R2 %04x",            bmphdr.Reserved2     ) ;
  ulSetError ( UL_DEBUG, "Offbits %08x",       bmphdr.OffBits       ) ;
  ulSetError ( UL_DEBUG, "Size %08x",          bmphdr.Size          ) ;
  ulSetError ( UL_DEBUG, "Width %08x",         bmphdr.Width         ) ;
  ulSetError ( UL_DEBUG, "Height %08x",        bmphdr.Height        ) ;
  ulSetError ( UL_DEBUG, "Planes %04x",        bmphdr.Planes        ) ;
  ulSetError ( UL_DEBUG, "Bitcount %04x",      bmphdr.BitCount      ) ;
  ulSetError ( UL_DEBUG, "Compression %08x",   bmphdr.Compression   ) ;
  ulSetError ( UL_DEBUG, "SizeImage %08x",     bmphdr.SizeImage     ) ;
  ulSetError ( UL_DEBUG, "XPelsPerMeter %08x", bmphdr.XPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "YPelsPerMeter %08x", bmphdr.YPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "ClrUsed %08x",       bmphdr.ClrUsed       ) ;
  ulSetError ( UL_DEBUG, "ClrImportant %08x",  bmphdr.ClrImportant  ) ;
#endif

  int isMonochrome = TRUE ;
  int isOpaque     = TRUE ;

  if ( bpp <= 8 )
  {
    for ( int i = 0 ; i < 256 ; i++ )
    {
      pal[i].b = tm -> readByte () ;
      pal[i].g = tm -> readByte () ;
      pal[i].r = tm -> readByte () ;

      /* According to BMP specs, this fourth value is not really alpha value
	 but just a filler byte, so it is ignored for now. */
      pal[i].a = tm -> readByte () ;
      //if ( pal[i].a != 255 ) isOpaque = FALSE ;

      if ( pal[i].r != pal[i].g ||
           pal[i].g != pal[i].b ) isMonochrome = FALSE ;
    }
  }

  fseek ( fp, bmphdr.OffBits, SEEK_SET ) ;

  bmphdr.SizeImage = w * h * (bpp / 8) ;
  GLubyte *data = new GLubyte [ bmphdr.SizeImage ] ;

  /* read and flip image */
  {
    int row_size = w * (bpp / 8) ;
    for ( int y = h-1 ; y >= 0 ; y-- )
    {
      GLubyte *row_ptr = &data [ y * row_size ] ;
      if ( fread ( row_ptr, 1, row_size, fp ) != (unsigned)row_size )
      {
        ulSetError ( UL_WARNING, "Premature EOF in '%s'", tm -> getPath () ) ;
        return ;
      }
    }
  }

  tm -> closeFile () ;

  GLubyte *image ;
  int z ;

  if ( bpp == 8 )
  {
    if ( isMonochrome )
      z = isOpaque ? 1 : 2 ;
    else
      z = isOpaque ? 3 : 4 ;

    image = new GLubyte [ w * h * z ] ;

    for ( int i = 0 ; i < w * h ; i++ )
      switch ( z )
      {
        case 1 : image [ i       ] = pal[data[i]].r ; break ;
        case 2 : image [ i*2     ] = pal[data[i]].r ;
                 image [ i*2 + 1 ] = pal[data[i]].a ; break ;
        case 3 : image [ i*3     ] = pal[data[i]].r ;
                 image [ i*3 + 1 ] = pal[data[i]].g ;
                 image [ i*3 + 2 ] = pal[data[i]].b ; break ;
        case 4 : image [ i*4     ] = pal[data[i]].r ;
                 image [ i*4 + 1 ] = pal[data[i]].g ;
                 image [ i*4 + 2 ] = pal[data[i]].b ;
                 image [ i*4 + 3 ] = pal[data[i]].a ; break ;
        default : break ;
      }

    delete data ;
  }
  else
  if ( bpp == 24 )
  {
    z = 3 ;
    image = data ;

    /* BGR --> RGB */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 3 * i ] ;
      image [ 3 * i ] = image [ 3 * i + 2 ];
      image [ 3 * i + 2 ] = tmp ;
    }
  }
  else
  if ( bpp == 32 )
  {
    z = 4 ;
    image = data ;

    /* BGRA --> RGBA */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 4 * i ] ;
      image [ 4 * i ] = image [ 4 * i + 2 ];
      image [ 4 * i + 2 ] = tmp ;
    }
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Can't load %d bpp BMP textures.", bpp ) ;
    tm -> loadDummy () ;
    return ;
  }

  tm -> setAlpha ( z == 4 ) ;
  tm -> make_mip_maps ( image, w, h, z ) ;
}


// This really simple (raw paletted) format is used by older MSFS for textures
void ssgLoadMDLTexture ( const char *fname )
{
  ssgTextureManager* tm = ssgTextureManager::get () ;
  
  FILE *tfile;
  if ( (tfile = fopen(fname, "rb")) == NULL) {
    ulSetError( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname );
    tm -> loadDummy();
    return;
  }
  
  fseek(tfile, 0, SEEK_END);
  unsigned long file_length = ftell(tfile);
  
  if (file_length != 65536) {
    // this is not a MSFS-formatted texture, so it's probably a BMP
    fclose(tfile);
    ssgLoadBMP( fname );
    return;
  } else {
    fseek(tfile, 0, SEEK_SET);
    
    unsigned char *texels = new unsigned char[256 * 256 * 4];
    int c = 0;
    for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
        unsigned char b;
        fread(&b, 1, 1, tfile);
        texels[c++] = fsTexPalette[b*4    ];
        texels[c++] = fsTexPalette[b*4 + 1];
        texels[c++] = fsTexPalette[b*4 + 2];
        texels[c++] = fsTexPalette[b*4 + 3];
      }
    }
    
    fclose(tfile);
    
    // _ssgAlphaFlag = true ; ??
    tm -> make_mip_maps ( texels, 256, 256, 4 ) ;
  }
}
