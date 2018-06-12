#pragma once

#include <iostream>
using namespace std;

#define MAX_VEC_DIR_NUM (5)

enum
{
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
	LOCAL,
	D_BUTT
		
};


#define VECTOR_UP (Vector2D (0, -1))
#define VECTOR_DOWN (Vector2D(0, 1))
#define VECTOR_LEFT (Vector2D(-1, 0))
#define VECTOR_RIGHT (Vector2D(1, 0))
#define VECTOR_ZERO (Vector2D(0,0))


struct Vector2D
{
	int col;
	int row;
	Vector2D():col(0), row(0){}
	Vector2D(int a, int b):col(a), row(b){}
	
	// Sets col and row to zero
	void Zero(){col=0;row=0;}
	void Set(int _col, int _row) {col=_col; row=_row;}
	bool isZero() const{return col == 0 && row == 0;}
	
	inline int LengthSq() const;
	
	inline bool isPerp(Vector2D &vec);
	
	inline Vector2D Perp()const;
	inline int DistanceSq(const Vector2D &v2) const;
	inline Vector2D GetReverse() const;
	
	const Vector2D& operator += (const Vector2D &rhs)
	{
		col += rhs.col;
		row += rhs.row;
		return *this;
	}
	
	const Vector2D& operator -=(const Vector2D &rhs)
	{
		
		col -= rhs.col;
		row -= rhs.row;
		return *this;
	}
	
	const Vector2D & operator *= (const int & rhs)
	{
		col *= rhs;
		row *= rhs;
		return *this;
		
	}
	
	const Vector2D & operator /=(const int &rhs)
	{
		col /= rhs;
		row /= rhs;
		return *this;
	}
	
	bool operator == (const Vector2D &rhs)const
	{
			return (col == rhs.col) &&(row == rhs.row);
	}
	
	bool operator != (const Vector2D&rhs) const
	{
		return (col != rhs.col) || (row != rhs.row);
	}
	
	bool operator < (const Vector2D &rhs) const 
	{
		
		if (col != rhs.col)
		{
			return col < rhs.col;
			
		}
		return row < rhs.row;
	}
	
	friend ostream & operator << (ostream & output, const Vector2D &a);
};

inline ostream& operator << (ostream & output , const Vector2D& a)
{
	output << "(" << a.col <<", " << a.row << ")" << std::endl;
	return output;
}

inline int Vector2D::LengthSq()const
{
	return (col * col + row *row);
}


inline Vector2D Vector2D::Perp()const 
{
	
	return Vector2D(-row, col);
}

inline int Vector2D::DistanceSq(const Vector2D &v2) const 
{
	int ySeparation = v2.row - row;
	int xSeparation = v2.col - col;
	
	return ySeparation *ySeparation + xSeparation*xSeparation;
}

inline Vector2D Vector2D::GetReverse() const
{
	return Vector2D(-this->col, -this->row);
}

bool Vector2D::isPerp(Vector2D &vec)
{
	if (this -> col*vec.col +this->row*vec.row == 0)
	{
		return true;
	}
	return false;
}

inline Vector2D operator*(const Vector2D &lhs, int rhs);
inline Vector2D operator*(int lhs, const Vector2D &rhs);
inline Vector2D operator-(const Vector2D &lhs, const Vector2D&rhs);
inline Vector2D operator+(const Vector2D&lhs, const Vector2D &rhs);
inline Vector2D operator/(const Vector2D &lhs, int rhs);

inline Vector2D operator*(const Vector2D &lhs, int rhs)
{
	Vector2D result(lhs);
	result *= rhs;
	return result;
}

inline Vector2D operator*(int lhs, const Vector2D &rhs)
{
	Vector2D result(rhs);
	result *=lhs;
	return result;
}

inline Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs)
{
	Vector2D result(lhs);
	result.col -= rhs.col;
	result.row -= rhs.row;
	return result;
}

inline Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs)
{
	
	Vector2D result(lhs);
	result.col += rhs.col;
	result.row += rhs.row;
	return result;
}

inline Vector2D operator/(const Vector2D & lhs, int val)
{
	Vector2D result(lhs);
	result.col /= val;
	result.row /= val;
	return result;
}

