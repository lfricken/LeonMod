/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#pragma once
#ifndef CVPOINT_H
#define CVPOINT_H
#define decimal float // safe decimal

class CvPoint3
{
public:
	CvPoint3(): x(0), y(0), z(0) {}
	CvPoint3(decimal fx, decimal fy, decimal fz) : x(fx),y(fy),z(fz) {}

	bool operator== (const CvPoint3& pt) const
	{
		return (x == pt.x && y == pt.y && z == pt.z);
	}

	inline CvPoint3 operator+ (const CvPoint3& pt) const
	{
		return CvPoint3(x+pt.x,y+pt.y,z+pt.z);
	}

	inline CvPoint3 operator- (const CvPoint3& pt) const
	{
		return CvPoint3(x-pt.x,y-pt.y,z-pt.z);
	}

	inline decimal operator* (const CvPoint3& pt) const
	{
		return x*pt.x+y*pt.y+z*pt.z;
	}

	inline CvPoint3 operator* (decimal fScalar) const
	{
		return CvPoint3(fScalar*x,fScalar*y,fScalar*z);
	}

	inline CvPoint3 operator/ (decimal fScalar) const
	{
		decimal fInvScalar = ((decimal)1) / fScalar;
		return CvPoint3(fInvScalar*x,fInvScalar*y,fInvScalar*z);
	}

	inline CvPoint3 operator- () const
	{
		return CvPoint3(-x,-y,-z);
	}

	inline decimal Length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline decimal Unitize()
	{
		decimal length = Length();
		if(length != 0)
		{
			x /= length;
			y /= length;
			z /= length;
		}
		return length;
	}

	decimal x, y, z;
};

#endif //CVPOINT_H
