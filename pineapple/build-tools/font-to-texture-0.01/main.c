
#include <stdio.h>
#include <stdlib.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <math.h>
#include <string>
#include <algorithm>
#include <cctype>
#define PNG_SKIP_SETJMP_CHECK 0
#include <sstream>
#include <png.h>

FT_Library library;
FT_Face face;

#define TEXTUREWIDTH 256
#define TEXTUREHEIGHT 256
#define FONTSIZE 20

#define SCRHEIGHT TEXTUREHEIGHT
#define SCRWIDTH TEXTUREWIDTH

void stoupper(std::string& s)
{
  std::string::iterator i = s.begin();
  std::string::iterator end = s.end();

  while (i != end) {
    *i = std::toupper((unsigned char)*i);
    ++i;
  }
}


typedef struct
{
	int x,y,x2,y2;
	float u,v,u2,v2;
} charbox;

charbox boxes[93];

int
main (int argc, char **argv)
{
	char usefulchars[93]=("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"$%&*(){}[]:;@'~#?/\\<>,.+-_+|");
	char *fontname;
	int error;
	int i,totalwidth,realtotalwidth;
	int curx,cury;
	FILE *fd;

	unsigned char *mybitmap;

	char charcode;
	int glyph_index;
	FT_GlyphSlot slot;
	FT_Bitmap bitmap;
	FT_Glyph  aglyph;
	FT_BBox bbox;

	char pixel;
	
	int cellwidth,cellheight,celladvance;
	int maxheight, maxwidth;
	int xmin,xmax,ymin,ymax;
	int maxarea,maxside;
	int x,y;
	float tflt;
	int bm_left,bm_top;
	int my_bmoffset;

	png_bytep row_pointers[SCRHEIGHT];
	png_structp png_ptr;
	png_infop info_ptr;




	fontname = argv[1];

	error = FT_Init_FreeType(&library);
	if(error) 
	{
		fprintf(stderr,"Failed to init Freetype library.\n");
		exit(1);
	} 
	
	error = FT_New_Face(library, fontname, 0, &face);
	if(error == FT_Err_Unknown_File_Format) 
	{
		fprintf(stderr,"Failed to load font (%s) - unknown font file format.\n",fontname);
		exit(1);
	} 
	else if (error)
	{
		fprintf(stderr,"Failed to load font (%s)\n",fontname);
		exit(1);
	}
	
	mybitmap=(unsigned char *)malloc(TEXTUREWIDTH*TEXTUREHEIGHT*3);
	if(!mybitmap)
	{
		fprintf(stderr,"Failed allocate bitmap\n");
		exit(1);
	}
	memset(mybitmap,0,TEXTUREWIDTH*TEXTUREHEIGHT*3);

	error = FT_Set_Char_Size(
				 face,    /* handle to face object           */
				 0,       /* char_width in 1/64th of points  */
				 FONTSIZE * 64, /* char_height in 1/64th of points */
				 100,     /* horizontal device resolution    */
				 100 );   /* vertical device resolution      */
	if(error) 
	{
		fprintf(stderr,"Failed to set font size (!?!)\n");
		exit(1);
	} 

	totalwidth = 0;
	realtotalwidth = 0;

	curx=0;
	cury=0;

	maxwidth = (face->bbox.xMax - face->bbox.xMin)>>6;
	maxheight = (face->bbox.yMax - face->bbox.yMin)>>6;
	maxarea = (maxwidth*maxheight*sizeof(usefulchars));
	maxside = sqrt(maxarea);

	/* the character is drawn *above* this point, so we need to make room... */
	cury += maxheight;

//	printf("Max cell size = %dx%d -> max area= %d pixels (%d square) for %d chars\n",maxwidth,maxheight,maxarea,maxside,sizeof(usefulchars));

  char *headername = "font-table.h";
  
  if(argc >= 4)
  {
  	headername = (char *)argv[3];
  }
	
	fd = fopen(headername,"w");

	std::string str = fontname;
	str = str.substr(6, str.size()-10);
	std::stringstream ss;
	ss << str;
	ss << "_H_";
	std::string header = ss.str();
	stoupper(header);
	fprintf (fd,"/*Do not edit this file.  It is machine generated. */\n\n#ifndef %s\n#define %s\n\nnamespace %s{\n\nstatic int TextureWidth=%d; \nstatic int TextureHeight=%d;\nstatic FONTTABLE font[] = \n{\n",
					 header.c_str(), header.c_str(), str.c_str(), TEXTUREWIDTH, TEXTUREHEIGHT);


	for(i=0;i<(sizeof(usefulchars)-1);i++)
	{	
		charcode = usefulchars[i];
		glyph_index = FT_Get_Char_Index(face, charcode);
		
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		
		/* just a shortcut */
		slot = face->glyph;

		error = FT_Render_Glyph(slot, ft_render_mode_normal);

		/* the glyph is now rendered into an internal buffer - we need a copy */


		/* first, let's get decent metrics for this glyph */
		FT_Get_Glyph(slot, &aglyph );

		/* get the measurements for this char */
		FT_Glyph_Get_CBox(aglyph,ft_glyph_bbox_pixels,&bbox);

		/* get the bitmap data out of face->glyph->bitmap */


		/* we want to track the xMin,xMax,yMin,yMax for each char */
		/* we can store the baseline offset for the whole font, 
			as long as they are all level */

		cellwidth = slot->metrics.width>>6;
		cellheight = slot->metrics.height>>6;
		celladvance = slot->advance.x >>6;

		totalwidth += celladvance;
		realtotalwidth += cellwidth;
		

		bitmap = slot->bitmap;
		bm_left =slot->bitmap_left;
		bm_top =slot->bitmap_top;

		
		/* figure out if the current char will fit on this row or not*/
		/* this *should* be bitmap.width, but that seems to be too small */
		if(curx > (TEXTUREWIDTH-celladvance))
		{
			curx = 0;
			cury += maxheight;
		}


		for(y=0;y<bitmap.rows;y++)
		{ 
			for(x=0;x<bitmap.width;x++)
			{
				pixel = bitmap.buffer[ y*bitmap.pitch + x ];
				my_bmoffset = curx + x + bm_left + (cury + y - bm_top)*TEXTUREWIDTH;
				mybitmap[my_bmoffset*3] = pixel;
				mybitmap[my_bmoffset*3+1] = pixel;
				mybitmap[my_bmoffset*3+2] = pixel;
			}
		}

		cellheight = bbox.yMax-bbox.yMin;

		xmin = curx + bbox.xMin + bm_left;
		xmax = curx + bbox.xMax + bm_left;
		ymin = cury - bm_top;
		ymax = cury + cellheight - bm_top;

		boxes[i].x= xmin;
		boxes[i].x2= xmax;		
		boxes[i].y= ymin;
		boxes[i].y2= ymax;

		tflt = ((float)xmin/(float)TEXTUREWIDTH);

		boxes[i].u = ((float)xmin/(float)TEXTUREWIDTH);
		boxes[i].u2 = ((float)xmax/(float)TEXTUREWIDTH);
		boxes[i].v = ((float)ymin/(float)TEXTUREWIDTH);
		boxes[i].v2 = ((float)ymax/(float)TEXTUREWIDTH);

#ifdef POOT
		if(charcode=='@' || charcode=='a' || charcode=='e')
		{
			printf("%c: (%d,%d)-(%d,%d) xminmax[%d,%d] yminmax[%d,%d] BITMAP=%dx%d OFFSET=(%d,%d)\n",charcode,
			       boxes[i].x,boxes[i].y,boxes[i].x2,boxes[i].y2,
			       bbox.xMin, bbox.xMax, bbox.yMin, bbox.yMax,
			       bitmap.width,bitmap.rows,
			       bm_left,bm_top
			       );
		}
#endif
		curx += celladvance;

//		printf ("%c %d,%d [adv=%d] {%d,%d}\n",charcode, cellwidth,cellheight,celladvance,curx,cury);
			
		fprintf (fd,"\t{'");
		if(charcode=='\\' || charcode=='\'')
		{
		fputc('\\',fd);
		}
		fprintf (fd,"%c', %d,%d,%d,%d,  %f,%f,%f,%f},\n",charcode,
			boxes[i].x,boxes[i].y,boxes[i].x2,boxes[i].y2,
			boxes[i].u,boxes[i].v,boxes[i].u2,boxes[i].v2);
	}

	fprintf (fd,"};\n");
	
	//write image data
	fprintf (fd,"\nstatic unsigned char fontimage[] = \n{");
	
	for(i=0; i<TEXTUREWIDTH*TEXTUREHEIGHT; i++) 
	{
		if(i%30 == 0) fprintf (fd, "\n");


				fprintf (fd, "%d,", mybitmap[i*3]);
	}
	
	fprintf (fd,"\n};\n}\n#endif\n");
	fclose(fd);

//	printf("Bitmap is %d wide, in total (%d really)\n",totalwidth,realtotalwidth);

//	fd = fopen("font.raw","wb");
//	fwrite(mybitmap,TEXTUREHEIGHT,TEXTUREWIDTH,fd);
//	fclose(fd);



	


	fd=fopen(argv[2],"wb");
	if(!fd) exit(2);

	png_ptr = png_create_write_struct
       (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
        NULL, NULL);
    if (!png_ptr) return (-1);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_write_struct(&png_ptr,
         (png_infopp)NULL);
       return (-1);
    }
	
	png_init_io(png_ptr, fd);

	png_set_IHDR(png_ptr, info_ptr, SCRWIDTH, SCRHEIGHT,
       8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	for(i=0;i<SCRHEIGHT;i++)
	{
		row_pointers[i]= &mybitmap[i*SCRWIDTH*3];
	}
	
	png_set_rows(png_ptr, info_ptr, (png_bytepp)&row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	//png_write_info(png_ptr, info_ptr);
	//png_write_image(png_ptr, row_pointers);
	//png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fd);

	return 0;
}
