// returns a colour for a given texture name.  currently only works with standard quake textures.
// expand as you see fit - you'll get the idea :-)

// V0.4 modifications
// - made some of the textures that were only used in medieval/metal maps orange instead of yellow
//   this may give spurious 'domination' warnings, but orange at least looks better - yellow is
//   really only suited to base maps
// - added rogue, hipnotic and zerstorer textures

#include <conio.h>
#include "tyrlite.h"

void FindTexlightColour (int *surf_r, int *surf_g, int *surf_b, char *texname)
{

if (nodefault == false) // [js] new feature
    {
	    if (!strncmp (texname, "*lava", 5))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strncmp (texname, "*slime", 6))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "+0butn"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "+0butnn"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "+0button"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "+0floorsw"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "+0planet"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "+0shoot"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "ceiling4"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "key03_1"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "tlight09"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "key03_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "ceil1_1"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "dem5_3"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "light1_1"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "wizwin1_2"))
	    {
		    *surf_r = 10;
		    *surf_g = 10;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "sfloor4_4"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "tech03_2"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "+0light01"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "tech06_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "light1_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "light1_3"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "tlight10"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "metal5_8"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "light1_5"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "carch03"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "tlight01"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "tlight02"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "light1_7"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "tlight03"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "light1_8"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "tlight05"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "basebutn3"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "window01_2"))
	    {
		    *surf_r = 10;
		    *surf_g = 10;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "window02_1"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "window01_3"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "sliplite"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "slip2"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "slipside"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "tlight07"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "window01_4"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "key01_1"))
	    {
		    *surf_r = 10;
		    *surf_g = 10;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "tlight01_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "key01_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "tele_top"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "*rift_1"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "htek04_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "smwpwin1"))
	    {
		    *surf_r = 255;
		    *surf_g = 255;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "sym08_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "sym10_2"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "tlight06"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "egypt11"))
	    {
		    *surf_r = 255;
		    *surf_g = 128;
		    *surf_b = 64;
	    }
	    else if (!strcmp (texname, "window2_1"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "window2_2"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "window2_3"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "window2_4"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strncmp (texname, "*blood", 6))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strncmp (texname, "*viscera", 8))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "kngygwin"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "meatfetus"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "musclite"))
	    {
		    *surf_r = 255;
		    *surf_g = 10;
		    *surf_b = 10;
	    }
	    else if (!strcmp (texname, "trim1_4"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
	    else if (!strcmp (texname, "trim_kn"))
	    {
		    *surf_r = 10;
		    *surf_g = 255;
		    *surf_b = 255;
	    }
        //++ [js] new feature
        else if (external == true) { 
            FindTexlightColourExt (surf_r, surf_g, surf_b, texname, tc_list);
        }
        //-- [js] new feature
	    else
	    {
		    // return a smaller value
		    *surf_r = 1;
		    *surf_g = 1;
		    *surf_b = 1;
	    }
    }
else  
    {
    //++ [js] new feature
    if (external == true) 
       {
       FindTexlightColourExt (surf_r, surf_g, surf_b, texname, tc_list);
       }  
    //-- [js] new feature
    else
       {
       // return a smaller value
       *surf_r = 1;
       *surf_g = 1;
       *surf_b = 1;
       }
    } 
}

//++ [js] new feature
void FindTexlightColourExt (int *surf_r, int *surf_g, int *surf_b, char *texname, tex_col_list list)
{
    int i,len,num;
    tex_col* entry;
    entry = NULL;
    num = list.num;
	
    // use default min values
    *surf_r = 1;
	*surf_g = 1;
	*surf_b = 1;
    
    // assign values based on external definition
    for (i = 1 ;i < num; i++) 
        {
        entry = &(list.entries[i]);
        len = strlen(entry->name);
        if((len > 0) && (strncmp (texname, entry->name, len) == 0))
	        {
		    *surf_r = entry->red;
		    *surf_g = entry->green;
		    *surf_b = entry->blue;

            break;
	        }
        }
    // sanity check
    /*
    if (*surf_r < 1) *surf_r = 1;
    if (*surf_g < 1) *surf_g = 1;
    if (*surf_b < 1) *surf_b = 1;
    */
}
//-- [js] new feature

void DecisionTime (char *msg)
{
	char c;

	// if we're forcing colouring irrespective of potential effectiveness (eg in a batch file)
	// just get out
	if (force)
		return;

	printf ("\nMHColour reports that it may not light this BSP effectively\n(%s)\n", msg);
	printf ("Continue? [Y/N] ");

	while (1)
	{
		c = getch ();

		if (c == 'y' || c == 'Y' || c == 'n' || c == 'N') break;
	}

	printf ("%c\n", c);

	if (c == 'n' || c == 'N') Error ("Program Terminated by user\n");
}

