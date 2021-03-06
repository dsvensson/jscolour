/*  Copyright (C) 1996-1997  Id Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

/*
 * tyrlite/ltface.c
 * Modifications by Kevin Shanahan, 1999-2000
 */

#include "tyrlite.h"

//#define MAX_BOUNCES 1000000
#define MAX_BOUNCES 1

entity_t bounce[MAX_BOUNCES];
int numbounces;

/*
 * ============
 * CastRay
 * Returns the distance between the points, or -1 if blocked
 * =============
 */
vec_t CastRay (vec3_t p1, vec3_t p2)
{
    int      i;
    vec_t    t;
    qboolean trace;

    trace = TestLine (p1, p2);

    if (!trace)
        return -1;        /* ray was blocked */

    t = 0;
    for (i=0 ; i< 3 ; i++)
        t += (p2[i]-p1[i]) * (p2[i]-p1[i]);

    if (t == 0)
        t = 1;            /* don't blow up... */
    return sqrt(t);
}

/*
 * ============================================================================
 * SAMPLE POINT DETERMINATION
 * void SetupBlock (dface_t *f) Returns with surfpt[] set
 *
 * This is a little tricky because the lightmap covers more area than the face.
 * If done in the straightforward fashion, some of the sample points will be
 * inside walls or on the other side of walls, causing false shadows and light
 * bleeds.
 *
 * To solve this, I only consider a sample point valid if a line can be drawn
 * between it and the exact midpoint of the face.  If invalid, it is adjusted
 * towards the center until it is valid.
 *
 * (this doesn't completely work)
 * ============================================================================
 */

#define SINGLEMAP (18*18*4)

typedef struct {
    vec_t   lightmaps[MAXLIGHTMAPS][SINGLEMAP];
    int     numlightstyles;
    vec_t   *light;
    vec_t   facedist;
    vec3_t  facenormal;

    int     numsurfpt;
    vec3_t  surfpt[SINGLEMAP];

    vec3_t  texorg;
    vec3_t  worldtotex[2];    /* s = (world - texorg) . worldtotex[0] */
    vec3_t  textoworld[2];    /* world = texorg + s * textoworld[0]   */

    vec_t   exactmins[2], exactmaxs[2];

    int     texmins[2], texsize[2];
    int     lightstyles[256];
    int     surfnum;
    dface_t *face;

    /* Colored lighting */
    vec3_t  lightmapcolours[MAXLIGHTMAPS][SINGLEMAP];

	// texture light colour modification
	char texname[16];
} lightinfo_t;


/*
 * ================
 * CalcFaceVectors
 * Fills in texorg, worldtotex. and textoworld
 * ================
 */
void CalcFaceVectors (lightinfo_t *l)
{
    texinfo_t *tex;
    int       i, j;
    vec3_t    texnormal;
    float     distscale;
    vec_t     dist;
    vec_t     len;

    tex = &texinfo[l->face->texinfo];

    /* convert from float to vec_t */
    for (i=0 ; i<2 ; i++)
        for (j=0 ; j<3 ; j++)
            l->worldtotex[i][j] = tex->vecs[i][j];

    /* calculate a normal to the texture axis.  points can be moved along */
    /* this without changing their S/T                                    */
    texnormal[0] = tex->vecs[1][1]*tex->vecs[0][2]
        - tex->vecs[1][2]*tex->vecs[0][1];
    texnormal[1] = tex->vecs[1][2]*tex->vecs[0][0]
        - tex->vecs[1][0]*tex->vecs[0][2];
    texnormal[2] = tex->vecs[1][0]*tex->vecs[0][1]
        - tex->vecs[1][1]*tex->vecs[0][0];
    VectorNormalize (texnormal);

    /* flip it towards plane normal */
    distscale = DotProduct (texnormal, l->facenormal);
    if (!distscale)
        Error ("Texture axis perpendicular to face\n"
               "Face point at (%f, %f, %f)\n",
               dvertexes[ dedges[ l->face->firstedge ].v[ 0 ] ].point[ 0 ],
               dvertexes[ dedges[ l->face->firstedge ].v[ 0 ] ].point[ 1 ],
               dvertexes[ dedges[ l->face->firstedge ].v[ 0 ] ].point[ 2 ]);
    if (distscale < 0) {
        distscale = -distscale;
        VectorSubtract (vec3_origin, texnormal, texnormal);
    }

    /* distscale is the ratio of the distance along the texture normal to */
    /* the distance along the plane normal                                */
    distscale = 1/distscale;

    for (i=0 ; i<2 ; i++) {
        len = VectorLength (l->worldtotex[i]);
        dist = DotProduct (l->worldtotex[i], l->facenormal);
        dist *= distscale;
        VectorMA (l->worldtotex[i], -dist, texnormal, l->textoworld[i]);
        VectorScale (l->textoworld[i], (1/len)*(1/len), l->textoworld[i]);
    }

    /* calculate texorg on the texture plane */
    for (i=0 ; i<3 ; i++)
        l->texorg[i] = -tex->vecs[0][3]* l->textoworld[0][i]
            - tex->vecs[1][3] * l->textoworld[1][i];

    /* project back to the face plane */
    dist = DotProduct (l->texorg, l->facenormal) - l->facedist - 1;
    dist *= distscale;
    VectorMA (l->texorg, -dist, texnormal, l->texorg);
}

/*
 * ================
 * CalcFaceExtents
 * Fills in s->texmins[] and s->texsize[]
 * also sets exactmins[] and exactmaxs[]
 * ================
 */
void CalcFaceExtents (lightinfo_t *l, vec3_t faceoffset)
{
    dface_t   *s;
    vec_t     mins[2], maxs[2], val;
    int       i,j, e;
    dvertex_t *v;
    texinfo_t *tex;

    s = l->face;

    mins[0] = mins[1] = 999999;
    maxs[0] = maxs[1] = -99999;

    tex = &texinfo[s->texinfo];

    for (i=0 ; i<s->numedges ; i++) 
	{
        e = dsurfedges[s->firstedge+i];
        if (e >= 0) v = dvertexes + dedges[e].v[0];
        else        v = dvertexes + dedges[-e].v[1];

        for (j=0 ; j<2 ; j++) 
		{
            val = (v->point[0]+faceoffset[0]) * tex->vecs[j][0]
                + (v->point[1]+faceoffset[1]) * tex->vecs[j][1]
                + (v->point[2]+faceoffset[2]) * tex->vecs[j][2]
                + tex->vecs[j][3];
            if (val < mins[j]) mins[j] = val;
            if (val > maxs[j]) maxs[j] = val;
        }
    }

    for (i=0 ; i<2 ; i++) 
	{
        l->exactmins[i] = mins[i];
        l->exactmaxs[i] = maxs[i];

        mins[i] = floor(mins[i]/16);
        maxs[i] = ceil(maxs[i]/16);

        l->texmins[i] = mins[i];
        l->texsize[i] = maxs[i] - mins[i];
        if (l->texsize[i] > 17)
            Error ("Bad surface extents");
    }
}

void CalcFaceExtentsNoErr (lightinfo_t *l, vec3_t faceoffset)
{
    dface_t   *s;
    vec_t     mins[2], maxs[2], val;
    int       i,j, e;
    dvertex_t *v;
    texinfo_t *tex;

    s = l->face;

    mins[0] = mins[1] = 999999;
    maxs[0] = maxs[1] = -99999;

    tex = &texinfo[s->texinfo];

    for (i=0 ; i<s->numedges ; i++) 
	{
        e = dsurfedges[s->firstedge+i];
        if (e >= 0) v = dvertexes + dedges[e].v[0];
        else        v = dvertexes + dedges[-e].v[1];

        for (j=0 ; j<2 ; j++) 
		{
            val = (v->point[0]+faceoffset[0]) * tex->vecs[j][0]
                + (v->point[1]+faceoffset[1]) * tex->vecs[j][1]
                + (v->point[2]+faceoffset[2]) * tex->vecs[j][2]
                + tex->vecs[j][3];
            if (val < mins[j]) mins[j] = val;
            if (val > maxs[j]) maxs[j] = val;
        }
    }

    for (i=0 ; i<2 ; i++) 
	{
        l->exactmins[i] = mins[i];
        l->exactmaxs[i] = maxs[i];

        mins[i] = floor(mins[i]/16);
        maxs[i] = ceil(maxs[i]/16);

        l->texmins[i] = mins[i];
        l->texsize[i] = maxs[i] - mins[i];
    }
}

/*
 * =================
 * CalcPoints
 * For each texture aligned grid point, back project onto the plane
 * to get the world xyz value of the sample point
 * =================
 */
int c_bad;
void CalcPoints (lightinfo_t *l)
{
    int    i;
    int    s, t, j;
    int    w, h, step;
    vec_t  starts, startt, us, ut;
    vec_t  *surf;
    vec_t  mids, midt;
    vec3_t facemid, move;

    /* fill in surforg                                         */
    /* the points are biased towards the center of the surface */
    /* to help avoid edge cases just inside walls              */

    surf = l->surfpt[0];
    mids = (l->exactmaxs[0] + l->exactmins[0])/2;
    midt = (l->exactmaxs[1] + l->exactmins[1])/2;

    for (j=0 ; j<3 ; j++)
        facemid[j] = l->texorg[j] + l->textoworld[0][j]*mids
            + l->textoworld[1][j]*midt;

    if (extrasamples) {    /* extra filtering */
        h = (l->texsize[1]+1)*2;
        w = (l->texsize[0]+1)*2;
        starts = (l->texmins[0]-0.5)*16;
        startt = (l->texmins[1]-0.5)*16;
        step = 8;
    } else {
        h = l->texsize[1]+1;
        w = l->texsize[0]+1;
        starts = l->texmins[0]*16;
        startt = l->texmins[1]*16;
        step = 16;
    }

    l->numsurfpt = w * h;
    for (t=0 ; t<h ; t++) {
        for (s=0 ; s<w ; s++, surf+=3) {
            us = starts + s*step;
            ut = startt + t*step;

            /* if a line can be traced from surf to facemid, point is good */
            for (i=0 ; i<6 ; i++) {
                /* calculate texture point */
                for (j=0 ; j<3 ; j++)
                    surf[j] = l->texorg[j] + l->textoworld[0][j]*us
                        + l->textoworld[1][j]*ut;

                if (CastRay (facemid, surf) != -1)
                    break;    /* got it */
                if (i & 1) {
                    if (us > mids) {
                        us -= 8;
                        if (us < mids)
                            us = mids;
                    } else {
                        us += 8;
                        if (us > mids)
                            us = mids;
                    }
                } else {
                    if (ut > midt) {
                        ut -= 8;
                        if (ut < midt)
                            ut = midt;
                    } else {
                        ut += 8;
                        if (ut > midt)
                            ut = midt;
                    }
                }

                /* move surf 8 pixels towards the center */
                VectorSubtract (facemid, surf, move);
                VectorNormalize (move);
                VectorMA (surf, 8, move, surf);
            }
            if (i == 2)
                c_bad++;
        }
    }
}


/*
 * ============================================================================
 * FACE LIGHTING
 * ============================================================================
 */

int c_culldistplane;
int c_proper;

/*
 * ==============================================
 * TYRLITE: Attenuation formulae setup functions
 * ==============================================
 */
float scaledDistance(float distance, entity_t *light)
{
    switch (light->formula) {
    case 1:
    case 2:
    case 3:
        /* Return a small distance to prevent culling these lights, since we */
        /* know these formulae won't fade to nothing.                        */
        return (distance<=0) ? -0.25 : 0.25;
    case 0:
        return scaledist * light->atten * distance;
    default:
        return 1;  /* shut up compiler warnings */
    }
}

float scaledLight(float distance, entity_t *light)
{
    float tmp = scaledist * light->atten * distance;
    switch (light->formula) {
    case 3: return light->light;
    case 1: return light->light / (tmp / 128);
    case 2: return light->light / ((tmp * tmp) / 16384);
    case 0:
        if (light->light > 0)
            return (light->light - tmp > 0) ? light->light - tmp : 0;
        else
            return (light->light + tmp < 0) ? light->light + tmp : 0;
    default:
        return 1;  /* shut up compiler warnings */
    }
}


/*
 * ================
 * SingleLightFace
 * ================
 */
void SingleLightFace (entity_t *light, lightinfo_t *l, vec3_t faceoffset, int bouncelight)
{
    vec_t    dist;
    vec3_t   incoming;
    vec_t    angle;
    vec_t    add;
    vec_t    *surf;
    qboolean hit;
    int      mapnum;
    int      size;
    int      c, i;
    vec3_t   rel;
    vec3_t   spotvec;
    vec_t    falloff;
    vec_t    *lightsamp;
    /* Colored lighting */
    vec3_t        *lightcoloursamp;

    VectorSubtract (light->origin, bsp_origin, rel);
    dist = scaledDistance((DotProduct(rel, l->facenormal) - l->facedist), light);

    /* don't bother with lights behind the surface */
    if (dist <= 0) return;

    /* don't bother with light too far away */
    if (dist > abs(light->light)) 
	{
        c_culldistplane++;
        return;
    }

    if (light->targetent) 
	{
        VectorSubtract (light->targetent->origin, light->origin, spotvec);
        VectorNormalize (spotvec);
        if (!light->angle)
            falloff = -cos(20*Q_PI/180);
        else
            falloff = -cos(light->angle/2*Q_PI/180);
    }
	else if (light->use_mangle) 
	{
        VectorCopy (light->mangle, spotvec);
        if (!light->angle)
            falloff = -cos(20*Q_PI/180);
        else
            falloff = -cos(light->angle/2*Q_PI/180);
    }
	else 
        falloff = 0;    /* shut up compiler warnings */

    mapnum = 0;
    for (mapnum=0 ; mapnum < l->numlightstyles ; mapnum++)
        if (l->lightstyles[mapnum] == light->style)
            break;

    lightsamp = l->lightmaps[mapnum];
    lightcoloursamp = l->lightmapcolours[mapnum];

    if (mapnum == l->numlightstyles) 
	{    /* init a new light map */
        size = (l->texsize[1]+1)*(l->texsize[0]+1);
        for (i=0 ; i<size ; i++) 
		{
            if (colored) 
			{
                lightcoloursamp[i][0] = 0;
                lightcoloursamp[i][1] = 0;
                lightcoloursamp[i][2] = 0;
            }
            lightsamp[i] = 0;
        }
    }

    /* check it for real */

    hit = false;
    c_proper++;

    surf = l->surfpt[0];

    for (c=0 ; c<l->numsurfpt ; c++, surf+=3) 
	{
        dist = scaledDistance(CastRay(light->origin, surf), light);

        if (dist < 0) continue;    /* light doesn't reach */

        VectorSubtract (light->origin, surf, incoming);
        VectorNormalize (incoming);
        angle = DotProduct (incoming, l->facenormal);
        if (light->targetent || light->use_mangle) 
		{    /* spotlight cutoff */
            if (DotProduct (spotvec, incoming) > falloff)
                continue;
        }

        angle = (1.0-scalecos) + scalecos*angle;
        add = scaledLight(CastRay(light->origin, surf), light);
        add *= angle;
        lightsamp[c] += add;

		// mfah - cap at 255 here
		if (lightsamp[c] > 255)
			lightsamp[c] = 255;

        if (colored) 
		{
            /* tQER<1>: Calculate add and keep in CPU register for faster */
            /* processing. x2.24 faster in profiler:                      */
            add /= 255.0f;

			lightcoloursamp[c][0] += (float)add * light->lightcolour[0];
			lightcoloursamp[c][1] += (float)add * light->lightcolour[1];
			lightcoloursamp[c][2] += (float)add * light->lightcolour[2];
        }

        if (abs(lightsamp[c]) > 1)        /* ignore really tiny lights */
            hit = true;
    }

    if (mapnum == l->numlightstyles && hit) 
	{
        if (mapnum == MAXLIGHTMAPS-1) 
		{
            printf ("WARNING: Too many light styles on a face\n");
            return;
        }

        l->lightstyles[mapnum] = light->style;
        l->numlightstyles++;    /* the style has some real data now */
    }
}

/*
 * =============
 * SkyLightFace
 * =============
 */
void SkyLightFace (lightinfo_t *l, vec3_t faceoffset)
{
    int    i,j;
	int	   k1, l1;
    vec_t  *surf;
    vec3_t incoming;
    vec_t  angle;

    /* Don't bother if surface facing away from sun */
    if (DotProduct (sunmangle, l->facenormal) <= 0)
        return;

    /* if sunlight is set, use a style 0 light map */
    for (i=0 ; i< l->numlightstyles ; i++)
        if (l->lightstyles[i] == 0)
            break;

    if (i == l->numlightstyles) 
	{
        if (l->numlightstyles == MAXLIGHTMAPS)
            return;        /* oh well, too many lightmaps... */
        l->lightstyles[i] = 0;
        l->numlightstyles++;
    }

    /* Check each point... */
    VectorCopy(sunmangle, incoming);
    VectorNormalize(incoming);
    angle = DotProduct (incoming, l->facenormal);
    angle = (1.0-scalecos) + scalecos*angle;
    surf = l->surfpt[0];
    for (j=0 ; j<l->numsurfpt ; j++, surf+=3) 
	{
        if (TestSky(surf, sunmangle)) 
		{
            l->lightmaps[i][j] += (angle*sunlight);
            if (colored) 
			{
                l->lightmapcolours[i][j][0] += (angle * sunlight * sunlight_color[0]) /255;
                l->lightmapcolours[i][j][1] += (angle * sunlight * sunlight_color[1]) /255;
                l->lightmapcolours[i][j][2] += (angle * sunlight * sunlight_color[2]) /255;
            }
        }
    }
}


/*
 * ============
 * FixMinlight
 * ============
 */
void FixMinlight (lightinfo_t *l)
{
    int   i, j, k;
    vec_t tmp;

    /* if minlight is set, there must be a style 0 light map */
    for (i=0 ; i< l->numlightstyles ; i++) 
	{
        if (l->lightstyles[i] == 0)
            break;
    }

    if (i == l->numlightstyles) 
	{
        if (l->numlightstyles == MAXLIGHTMAPS)
            return;        /* oh well.. */

        for (j=0 ; j<l->numsurfpt ; j++)
            l->lightmaps[i][j] = worldminlight;

        if (colored)
            for (j=0 ; j<l->numsurfpt ; j++) 
			{
                l->lightmapcolours[i][j][0] = (worldminlight * minlight_color[0]) /255;
                l->lightmapcolours[i][j][1] = (worldminlight * minlight_color[1]) /255;
                l->lightmapcolours[i][j][2] = (worldminlight * minlight_color[2]) /255;
            }

        l->lightstyles[i] = 0;
        l->numlightstyles++;
    }
	else 
	{
        for (j=0 ; j<l->numsurfpt ; j++) 
		{
            if ( l->lightmaps[i][j] < worldminlight)
                l->lightmaps[i][j] = worldminlight;
            if (colored) 
			{
                for (k=0 ; k<3 ; k++) 
				{
                    tmp = (vec_t)(worldminlight * minlight_color[k]) / 255.0f;
                    if (l->lightmapcolours[i][j][k] < tmp )
                        l->lightmapcolours[i][j][k] = tmp;
                }
            }
        }
    }
}


/*
 * To (help) make the code below in SingleLightFace more readable, I'll use
 * these macros.
 *
 * light is the light intensity, needed to check if +ve or -ve.
 * src and dest are the source and destination color vectors (vec3_t).
 * dest becomes a copy of src where
 *    MakePosColoredLight removes negative light components.
 *    MakeNegColoredLight removes positive light components.
 */
#define MakePosColoredLight(light, dest, src)    \
if (light >= 0) {                                \
    for (k=0 ; k<3 ; k++)                        \
        if (src[k] < 0) dest[k] = 0;             \
        else            dest[k] = src[k];        \
} else {                                         \
    for (k=0 ; k<3 ; k++)                        \
        if (src[k] > 0) dest[k] = 0;             \
        else            dest[k] = src[k];        \
}

#define MakeNegColoredLight(light, dest, src)    \
if (light >= 0) {                                \
    for (k=0 ; k<3 ; k++)                        \
        if (src[k] > 0) dest[k] = 0;             \
        else            dest[k] = src[k];        \
} else {                                         \
    for (k=0 ; k<3 ; k++)                        \
        if (src[k] < 0) dest[k] = 0;             \
        else            dest[k] = src[k];        \
}


/*
 * ============
 * LightFace
 * ============
 */
void LightFace (int surfnum, qboolean nolight, vec3_t faceoffset)
{
    dface_t     *f;
    lightinfo_t l;
    int         s, t;
    int         i,j,c;

    /* TYR - temp vars */
    vec_t       max;
    int         x1,x2,x3,x4;
    int         k; /* for the macros */

    vec_t       total;
    int         size;
    int         lightmapwidth;
    int         lightmapsize;
    byte        *out;
    vec_t       *light;

    /* TYR - colored lights */
    vec3_t      *lightcolour;
    vec3_t      totalcolours;

    int         w, h;
    vec3_t      point;


    f = dfaces + surfnum;

    /* some surfaces don't need lightmaps */
    f->lightofs = -1;

    for (j=0 ; j<MAXLIGHTMAPS ; j++)
        f->styles[j] = 255;

    if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
        return;  /* non-lit texture */

    memset (&l, 0, sizeof(l));
    l.surfnum = surfnum;
    l.face = f;

    /* rotate plane */

    VectorCopy (dplanes[f->planenum].normal, l.facenormal);
    l.facedist = dplanes[f->planenum].dist;
    VectorScale (l.facenormal, l.facedist, point);
    VectorAdd( point, faceoffset, point );
    l.facedist = DotProduct( point, l.facenormal );

    if (f->side) 
	{
        VectorSubtract (vec3_origin, l.facenormal, l.facenormal);
        l.facedist = -l.facedist;
    }

    CalcFaceVectors (&l);
    CalcFaceExtents (&l, faceoffset);
    CalcPoints (&l);

    lightmapwidth = l.texsize[0]+1;

    size = lightmapwidth*(l.texsize[1]+1);
    if (size > SINGLEMAP)
        Error ("Bad lightmap size");

    for (i=0 ; i<MAXLIGHTMAPS ; i++)
        l.lightstyles[i] = 255;

    l.numlightstyles = 0;

	sprintf (l.texname, "%s", miptex[texinfo[f->texinfo].miptex].name);

	for (i=0 ; i<num_entities ; i++)
		if (entities[i].light)
			SingleLightFace (&entities[i], &l, faceoffset, 0);

    /* Minimum lighting */
    FixMinlight (&l);

    if (!l.numlightstyles)
        return;    /* no light hitting it */

    /* save out the values */

    for (i=0 ; i <MAXLIGHTMAPS ; i++)
        f->styles[i] = l.lightstyles[i];

    /* Extra room for RGBA lightmaps */
    if (colored) lightmapsize = size*l.numlightstyles*4;
    else         lightmapsize = size*l.numlightstyles;

    out = GetFileSpace (lightmapsize);
    f->lightofs = out - filebase;

    /* extra filtering */
    h = (l.texsize[1]+1)*2;
    w = (l.texsize[0]+1)*2;

    for (i=0 ; i< l.numlightstyles ; i++) 
	{
        if (l.lightstyles[i] == 0xff)
            Error ("Wrote empty lightmap");

        light = l.lightmaps[i];
        lightcolour = l.lightmapcolours[i];
        c = 0;

        for (t=0 ; t<=l.texsize[1] ; t++)
            for (s=0 ; s<=l.texsize[0] ; s++, c++) 
			{
                if (extrasamples) 
				{
                    x1 = t*2*w+s*2;
                    x2 = x1+1;
                    x3 = (t*2+1)*w+s*2;
                    x4 = x3+1;

                    /* filtered sample */
					total = light[x1] + light[x2] + light[x3] + light[x4];
					total *= 0.25;

                    /* Calculate the colour */
                    if (colored) 
					{
                        totalcolours[0] = lightcolour[x1][0] + lightcolour[x2][0] + lightcolour[x3][0] + lightcolour[x4][0];
                        totalcolours[0] *= 0.25;
                        totalcolours[1] = lightcolour[x1][1] + lightcolour[x2][1] + lightcolour[x3][1] + lightcolour[x4][1];
                        totalcolours[1] *= 0.25;
                        totalcolours[2] = lightcolour[x1][2] + lightcolour[x2][2] + lightcolour[x3][2] + lightcolour[x4][2];
                        totalcolours[2] *= 0.25;
                    }
                } 
				else 
				{
                    total = light[c];
                    if (colored)
                        VectorCopy (lightcolour[c], totalcolours);
                }

                total *= rangescale;    /* scale before clamping */

                /* CSL - Scale back intensity, instead of capping individual */
                /*       colours                                             */
                if (colored) 
				{
                    VectorScale (totalcolours, rangescale, totalcolours);
                    max = 0.0;

                    for (j = 0; j < 3; j++)
                        if (totalcolours[j] > max) 
						{
                            max = totalcolours[j];
                        } 
						else if (totalcolours[j] < 0.0f) 
						{
                            totalcolours[j] = 0.0f;	// this used to be an error!!!!
                        }
                    if (max > 255.0f)
                        VectorScale (totalcolours, 255.0f / max, totalcolours);
                }

                if (total > 255.0f)
                    total = 255.0f;
                else if (total < 0.0f)
					total = 0.0f;	// this used to be an error!!!

                /* Write out the lightmap in RGBA format */
                if (colored) 
				{
                    *out++ = totalcolours[0];
                    *out++ = totalcolours[1];
                    *out++ = totalcolours[2];
                }

				// not used in darkplaces - only in bsp 30
				*out++ = total;
            }
    }
}


extern int faces_ltoffset[MAX_MAP_FACES];
extern byte newdlightdata[MAX_MAP_LIGHTING];
extern int newlightdatasize;

void LightFaceLIT (int surfnum, qboolean nolight, vec3_t faceoffset)
{
    dface_t     *f;
    lightinfo_t l;
    int         s, t;
    int         i,j,c;

    /* TYR - temp vars */
    vec_t       max;
    int         x1,x2,x3,x4;
    int         k; /* for the macros */

    vec_t       total;
    int         size;
    int         lightmapwidth;
    int         lightmapsize;
    byte        *out;
    vec_t       *light;

    /* TYR - colored lights */
    vec3_t      *lightcolour;
    vec3_t      totalcolours;

    int         w, h;
    vec3_t      point;


    f = dfaces + surfnum;

	// this version already has the light offsets calculated from the original lighting, so we
	// will just reuse them.
    if (f->lightofs == -1)
		return;

    for (j=0 ; j<MAXLIGHTMAPS ; j++)
        f->styles[j] = 255;

    if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
        return;  /* non-lit texture */

    memset (&l, 0, sizeof(l));
    l.surfnum = surfnum;
    l.face = f;

    /* rotate plane */
    VectorCopy (dplanes[f->planenum].normal, l.facenormal);
    l.facedist = dplanes[f->planenum].dist;
    VectorScale (l.facenormal, l.facedist, point);
    VectorAdd( point, faceoffset, point );
    l.facedist = DotProduct( point, l.facenormal );

    if (f->side) 
	{
        VectorSubtract (vec3_origin, l.facenormal, l.facenormal);
        l.facedist = -l.facedist;
    }

    CalcFaceVectors (&l);
    CalcFaceExtents (&l, faceoffset);
    CalcPoints (&l);

    lightmapwidth = l.texsize[0]+1;

    size = lightmapwidth*(l.texsize[1]+1);
    if (size > SINGLEMAP)
        Error ("Bad lightmap size");

    for (i=0 ; i<MAXLIGHTMAPS ; i++)
        l.lightstyles[i] = 255;

    l.numlightstyles = 0;

	sprintf (l.texname, "%s", miptex[texinfo[f->texinfo].miptex].name);

	for (i=0 ; i<num_entities ; i++)
		if (entities[i].light)
			SingleLightFace (&entities[i], &l, faceoffset, 0);

    /* Minimum lighting */
    FixMinlight (&l);

    if (!l.numlightstyles)
        return;    /* no light hitting it */

    /* save out the values */

    for (i=0 ; i <MAXLIGHTMAPS ; i++)
        f->styles[i] = l.lightstyles[i];

    /* Extra room for RGBA lightmaps */
	lightmapsize = size*l.numlightstyles*3;

	// we have to store the new light data at the same offset as the old stuff...
    out = &newdlightdata[faces_ltoffset[surfnum]]; // GetFileSpace (lightmapsize);
	//f->lightofs = out - filebase;

    /* extra filtering */
    h = (l.texsize[1]+1)*2;
    w = (l.texsize[0]+1)*2;

    for (i=0 ; i< l.numlightstyles ; i++) 
	{
        if (l.lightstyles[i] == 0xff)
            Error ("Wrote empty lightmap");

        light = l.lightmaps[i];
        lightcolour = l.lightmapcolours[i];
        c = 0;

        for (t=0 ; t<=l.texsize[1] ; t++)
            for (s=0 ; s<=l.texsize[0] ; s++, c++) 
			{
                if (extrasamples) 
				{
                    x1 = t*2*w+s*2;
                    x2 = x1+1;
                    x3 = (t*2+1)*w+s*2;
                    x4 = x3+1;

                    /* Calculate the colour */
                    totalcolours[0] = lightcolour[x1][0] + lightcolour[x2][0] + lightcolour[x3][0] + lightcolour[x4][0];
                    totalcolours[0] *= 0.25;
                    totalcolours[1] = lightcolour[x1][1] + lightcolour[x2][1] + lightcolour[x3][1] + lightcolour[x4][1];
                    totalcolours[1] *= 0.25;
                    totalcolours[2] = lightcolour[x1][2] + lightcolour[x2][2] + lightcolour[x3][2] + lightcolour[x4][2];
                    totalcolours[2] *= 0.25;
                } 
				else 
				{
                    VectorCopy (lightcolour[c], totalcolours);
                }

                /* CSL - Scale back intensity, instead of capping individual */
                /*       colours                                             */
                    VectorScale (totalcolours, rangescale, totalcolours);
                    max = 0.0;

                for (j = 0; j < 3; j++)
                    if (totalcolours[j] > max) 
					{
                        max = totalcolours[j];
                    } 
					else if (totalcolours[j] < 0.0f) 
					{
                        totalcolours[j] = 0.0f;	// this used to be an error!!!!
                    }
                if (max > 255.0f)
                    VectorScale (totalcolours, 255.0f / max, totalcolours);

                /* Write out the lightmap in RGBA format */
                *out++ = totalcolours[0];
                *out++ = totalcolours[1];
                *out++ = totalcolours[2];
            }
    }
}


void TestSingleLightFace (entity_t *light, lightinfo_t *l, vec3_t faceoffset, int bouncelight)
{
    vec_t    dist;
    vec3_t   incoming;
    vec_t    angle;
    vec_t    add;
    vec_t    *surf;
	vec_t	 *lastsurf;
    qboolean hit;
    int      mapnum;
    int      size;
    int      c, i;
    vec3_t   rel;
    vec3_t   spotvec;
    vec_t    falloff;
    vec_t    *lightsamp;
    /* Colored lighting */
    vec3_t   *lightcoloursamp;

	int surf_r;
	int surf_g;
	int surf_b;

    VectorSubtract (light->origin, bsp_origin, rel);
    dist = scaledDistance((DotProduct(rel, l->facenormal) - l->facedist), light);

    /* don't bother with lights behind the surface */
    if (dist <= 0) return;

    /* don't bother with light too far away */
    if (dist > abs(light->light)) 
	{
        c_culldistplane++;
        return;
    }

	// mfah - find the light colour based on the surface name
	//++ [js] new feature
    FindTexlightColour (&surf_r, &surf_g, &surf_b, l->texname);
    /*
    if (nodefault != true) FindTexlightColour (&surf_r, &surf_g, &surf_b, l->texname);
    if (external == true) FindTexlightColourExt (&surf_r, &surf_g, &surf_b, l->texname, tc_list);   
    */
    //-- [js] new feature
 	
    surf = l->surfpt[0];

	// we could speed the whole thing up drastically by checking only the first and last point of
	// each face - trouble is, any large faces may have a light that only hits the middle.
    for (c=0 ; c<l->numsurfpt ; c++, surf+=3) 
	{
        dist = scaledDistance(CastRay(light->origin, surf), light);

        if (dist < 0) continue;    /* light doesn't reach */

        add = scaledLight(CastRay(light->origin, surf), light);

		if (add < (light->light / 3)) 
			continue;

		// normal light - other lights already have a colour assigned 
		// to them from when they were initially loaded
		// this will give madly high colour values here so we will scale them down later on
		light->lightcolour[0] = light->lightcolour[0] + surf_r;
		light->lightcolour[1] = light->lightcolour[1] + surf_g;
		light->lightcolour[2] = light->lightcolour[2] + surf_b;

		// speed up the checking process some more - if we have one hit on a face, all other hits
		// on the same face are just going to give the same result - so we can return now.
		return;
	}
}


void TestLightFace (int surfnum, qboolean nolight, vec3_t faceoffset)
{
    dface_t     *f;
    lightinfo_t l;
    int         s, t;
    int         i,j,c;

    /* TYR - temp vars */
    vec_t       max;
    int         x1,x2,x3,x4;
    int         k; /* for the macros */

    vec_t       total;
    int         size;
    int         lightmapwidth;
    int         lightmapsize;
    byte        *out;
    vec_t       *light;

    /* TYR - colored lights */
    vec3_t      *lightcolour;
    vec3_t      totalcolours;

    int         w, h;
    vec3_t      point;


    f = dfaces + surfnum;

    memset (&l, 0, sizeof(l));

	sprintf (l.texname, "%s", miptex[texinfo[f->texinfo].miptex].name);

	// we can speed up the checking process by ignoring any textures that give white light
	// this hasn't been done since version 0.2 - we can get rid of it
	//FindTexlightColour (&i, &j, &c, l.texname);

	//if (i == 255 && j == 255 && c == 255) return;

	// don't even bother with sky - although we might later on if we can get some kinda good
	// sky textures going.
	if (!strncmp (l.texname, "sky", 3))
		return;

    l.surfnum = surfnum;
    l.face = f;

    /* rotate plane */

    VectorCopy (dplanes[f->planenum].normal, l.facenormal);
    l.facedist = dplanes[f->planenum].dist;
    VectorScale (l.facenormal, l.facedist, point);
    VectorAdd( point, faceoffset, point );
    l.facedist = DotProduct( point, l.facenormal );

    if (f->side) 
	{
        VectorSubtract (vec3_origin, l.facenormal, l.facenormal);
        l.facedist = -l.facedist;
    }

    CalcFaceVectors (&l);

	// use the safe version here which will not give bad surface extents on special textures
    CalcFaceExtentsNoErr (&l, faceoffset);

    CalcPoints (&l);

	for (i=0 ; i<num_entities ; i++)
	{
		if (!strcmp (entities[i].classname, "light"))
		{
			// don't test torches, flames and globes - they already have their own light
			TestSingleLightFace (&entities[i], &l, faceoffset, 0);
		}
		else if (!strncmp (entities[i].classname, "light_fluor", 11))
		{
			// test fluoros as well
			TestSingleLightFace (&entities[i], &l, faceoffset, 0);
		}
	}
}
