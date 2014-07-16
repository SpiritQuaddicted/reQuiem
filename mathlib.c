/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// mathlib.c -- math primitives

#include "quakedef.h"

vec3_t	vec3_origin = {0, 0, 0};

int	_mathlib_temp_int1, _mathlib_temp_int2;
float	_mathlib_temp_float1, _mathlib_temp_float2;
vec3_t	_mathlib_temp_vec1;

void ProjectPointOnPlane (vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float	d, inv_denom;
	vec3_t	n;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	VectorScale (normal, inv_denom, n);
	VectorMA (p, -d, n, dst);
}

void VectorVectors (const vec3_t forward, vec3_t right, vec3_t up)
{
	PerpendicularVector (right, forward);
	CrossProduct (right, forward, up);
}

// JDH: This version of PerpendicularVector doesn't work properly on Linux
//      I reverted to Quake/Quake2 version

/*
void PerpendicularVector (vec3_t dst, const vec3_t src)
{
	if (!src[0])
	{
		VectorSet (dst, 1, 0, 0);
	}
	else if (!src[1])
	{
		VectorSet (dst, 0, 1, 0);
	}
	else if (!src[2])
	{
		VectorSet (dst, 0, 0, 1);
	}
	else
	{
		VectorSet (dst, -src[1], src[0], 0);
		VectorNormalizeFast (dst);
	}
}
*/

// assumes "src" is normalized
void PerpendicularVector (vec3_t dst, const vec3_t src)
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

// find the smallest magnitude axially aligned vector
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabs (src[i]) < minelem)
		{
			pos = i;
			minelem = fabs (src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

// project the point onto the plane defined by src
	ProjectPointOnPlane (dst, tempvec, src);

// normalize the result
	VectorNormalize (dst);
}

void RotatePointAroundVector (vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
	float	t0, t1, angle, c, s;
	vec3_t	vr, vu, vf;

	angle = DEG2RAD(degrees);
	c = cos(angle);
	s = sin(angle);
	VectorCopy (dir, vf);
	VectorVectors (vf, vr, vu);

	t0 = vr[0] * c + vu[0] * -s;
	t1 = vr[0] * s + vu[0] * c;
	dst[0] = (t0 * vr[0] + t1 * vu[0] + vf[0] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[0] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[0] * vf[2]) * point[2];

	t0 = vr[1] * c + vu[1] * -s;
	t1 = vr[1] * s + vu[1] * c;
	dst[1] = (t0 * vr[0] + t1 * vu[0] + vf[1] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[1] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[1] * vf[2]) * point[2];

	t0 = vr[2] * c + vu[2] * -s;
	t1 = vr[2] * s + vu[2] * c;
	dst[2] = (t0 * vr[0] + t1 * vu[0] + vf[2] * vf[0]) * point[0]
		+ (t0 * vr[1] + t1 * vu[1] + vf[2] * vf[1]) * point[1]
		+ (t0 * vr[2] + t1 * vu[2] + vf[2] * vf[2]) * point[2];
}

#ifdef _WIN32
#pragma optimize( "", off )
#endif
// JDH: this is a version of RotatePointAroundVector based on the Quake/Quake2 version.
//      But it seems the problem was in PerpendicularVector, so above version can stay.
/*
void RotatePointAroundVector (vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	float	angle, c, s;
	int		i;

	PerpendicularVector (m[0], dir);
	CrossProduct (m[0], dir, m[1]);

	VectorCopy (dir, m[2]);

//	memcpy (im, m, sizeof(im));
	im[0][0] = m[0][0];
	im[1][1] = m[1][1];
	im[2][2] = m[2][2];

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	angle = DEG2RAD (degrees);
	c = cos (angle);
	s = sin (angle);

	memset (zrot, 0, sizeof(zrot));
	zrot[2][2] = 1.0F;

	angle = DEG2RAD (degrees);
	c = cos (angle);
	s = sin (angle);

	zrot[0][0] = c;
	zrot[0][1] = s;
	zrot[1][0] = -s;
	zrot[1][1] = c;

	R_ConcatRotations (m, zrot, tmpmat);
	R_ConcatRotations (tmpmat, im, rot);

	for ( i = 0; i < 3; i++ )
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}
*/
#ifdef _WIN32
#pragma optimize( "", on )
#endif

/*
==================
BOPS_Error

Split out like this for ASM to call.
==================
*/
void BOPS_Error (void)
{
	Sys_Error ("BoxOnPlaneSide: Bad signbits");
}

#if !id386

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, const mplane_t *p)
{
	switch (p->signbits)
	{
	default:
	case 0:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 1:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 2:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 3:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) < p->dist) << 1));

	case 4:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 5:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 6:
		return	(((p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));

	case 7:
		return	(((p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2]) >= p->dist) |
				(((p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2]) < p->dist) << 1));
	}
}

#endif

void vectoangles (const vec3_t vec, vec3_t ang)
{
	float	forward, yaw, pitch;

	if (!vec[1] && !vec[0])
	{
		yaw = 0;
		pitch = (vec[2] > 0) ? 90 : 270;
	}
	else
	{
		yaw = atan2 (vec[1], vec[0]) * 180 / M_PI;
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch = atan2 (vec[2], forward) * 180 / M_PI;
		if (pitch < 0)
			pitch += 360;
	}

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float	angle, sr, sp, sy, cr, cp, cy, temp;

	if (angles[YAW])
	{
		angle = DEG2RAD(angles[YAW]);
		sy = sin(angle);
		cy = cos(angle);
	}
	else
	{
		sy = 0;
		cy = 1;
	}

	if (angles[PITCH])
	{
		angle = DEG2RAD(angles[PITCH]);
		sp = sin(angle);
		cp = cos(angle);
	}
	else
	{
		sp = 0;
		cp = 1;
	}

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right || up)
	{
		if (angles[ROLL])
		{
			angle = DEG2RAD(angles[ROLL]);
			sr = sin(angle);
			cr = cos(angle);

			if (right)
			{
				temp = sr * sp;
				right[0] = -1 * temp * cy + cr * sy;
				right[1] = -1 * temp * sy - cr * cy;
				right[2] = -1 * sr * cp;
			}

			if (up)
			{
				temp = cr * sp;
				up[0] = (temp * cy + sr * sy);
				up[1] = (temp * sy - sr * cy);
				up[2] = cr * cp;
			}
		}
		else
		{
			if (right)
			{
				right[0] = sy;
				right[1] = -cy;
				right[2] = 0;
			}

			if (up)
			{
				up[0] = sp * cy ;
				up[1] = sp * sy;
				up[2] = cp;
			}
		}
	}
}

vec_t VectorLength (const vec3_t v)
{
	float	length;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return sqrt(length);
}

float VectorNormalize (vec3_t v)
{
	float	length;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if (length)
		VectorScale (v, 1 / length, v);

	return length;
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (const float in1[3][3], const float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

/*
===================
FloorDivMod

Returns mathematically correct (floor-based) quotient and remainder for
numer and denom, both of which should contain no fractional part. The
quotient must fit in 32 bits.
====================
*/
void FloorDivMod (double numer, double denom, int *quotient, int *rem)
{
	int	q, r;
	double	x;

#ifndef PARANOID
	if (denom <= 0.0)
		Sys_Error ("FloorDivMod: bad denominator %d\n", denom);

//	if ((floor(numer) != numer) || (floor(denom) != denom))
//		Sys_Error ("FloorDivMod: non-integer numer or denom %f %f\n",
//				numer, denom);
#endif

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floor(numer - (x * denom));
	}
	else
	{
	// perform operations with positive values, and fix mod to make floor-based
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floor(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}

/*
===================
GreatestCommonDivisor
====================
*/
int GreatestCommonDivisor (int i1, int i2)
{
	if (i1 > i2)
	{
		if (i2 == 0)
			return (i1);
		return GreatestCommonDivisor (i2, i1 % i2);
	}
	else
	{
		if (i1 == 0)
			return (i2);
		return GreatestCommonDivisor (i1, i2 % i1);
	}
}

#if !id386

/*
===================
Invert24To16

Inverts an 8.24 value to a 16.16 value
====================
*/

fixed16_t Invert24To16 (fixed16_t val)
{
	if (val < 256)
		return (0xFFFFFFFF);

	return (fixed16_t)(((double)0x10000 * (double)0x1000000 / (double)val) + 0.5);
}

#endif

#ifdef HEXEN2_SUPPORT
#if !defined(_WIN32) || !defined(_MSC_VER)
float fastfabs(float _X)
{
	*((long*)&_X) &= 0x7fffffff;
	return (_X);
}
#endif
#endif
