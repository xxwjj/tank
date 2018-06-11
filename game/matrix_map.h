#pragma once

#include <iostream>
#include "define.h"
#include "constants.h"
#include "vector2d.h"
#inlucde <cassert>
#include <string>
#include <iostream>
#include <fsStream>
#include <vector>

typedef enum{
	E_FLAT = 0,
	E_WALL = 1,
	E_BRICK = 2,
	E_RIVER = 3,
	E_FOE = 4,
	E_FRIEND = 5,
	E_DIAMOND = 6,
	E_STAR = 7,
	E_BUTT
} E_CELL;

typedef enum
{
	NO_BULLET = 0,
	FOE_BULLET,
	OWE_BULLET,
} E_BULLET;

class cell
{
	public:
	E_CELL _type;
	E_BULLET _bullet;
	Vector2D _point;
	
	bool _deleted;
	Cell::Cell(void)
	{
		_point.Zero();
		_type = E_FLAT;
		_deleted = false;
	}
};


class MatrixMap
{
	protected:
	public:
	int _width;
	int _height;
	Cell * _cells;
	std::vector<Vector2D> _starVec;
	std::vector<Vector2D> _diamondVec;
	public:
	MatrixMap(void):_width(0), height(0), _cells(NULL) {
		
	}
	~MatrixMap(void) {m_free();}
	
	bool m_alloc(int col, int row);
	void m_free()
	{
		if (_cells != NULL)
		{
			delete [] _cells;
			_cells = NULL;
		}
	}
	int coord2index(int col, int row) {return row*_width + col;}
	int pos2Index(Vector2D pos) {return coord2index(pos.col, pos.row);}
	int index2col(int id){return id % _width;}
	int index2row(int id) {return id / _width;}
	Vector2D index2Pos(int id) {return Vector2D(index2col(id), index2row(id));}
	void clearVolatileTerrain();
	void clearVolatileTerrain(E_CELL type);
	void clearDeleteTerrain();
	void clone(MatrixMap & newMap);
	
	int getNearestTerrainDistance(Vector2D &pos, Vector2D &dir, E_CELL type);
	int getTerrainNumInVision(Vector2D &ops, Vector2D &dir, E_CELL type);
	int getTerrainNumInVision(Vector2D &ops, E_CELL type);
	
	int getTerrainNumINDistance(Vector2D &ops, E_CELL type);
	
	int getPlayerNumInOneStep(Vector2D &pos);
	
	bool isValideCoord(int col, int row)
	{
		if (((col >= 0) && (col < _width)) && ((row >= 0) && (row < _height)))
		{
			return true;
		}
		return false;
		
	}
	bool isValideCoord(Vector2D &ops)
	{
		if (((pos.col >= 0) && (pos.col <_width)) && ((pos.row >= 0) && (pos.row < _height)))
		{
			return true;
		}
		return false;
	}
	Vector2D * getPoint(int col, int row)
	{
		if (!isValideCoord(col, row))
		{
			return NULL;
			
		}
		return &(_cells+coord2index(col, row))->_point;
		
	}
	
	void setTerrain(int col, int row, E_CELL type)
	{
		if (isValideCoord(col, row))
		{
			return ;
		}
		return (_cells+coord2index(col, row))->_type;
	}
	
	int getTerrain(int col, int row)
	{
		if (!isValideCoord(col, row))
		{
			return E_WALL;
		}
		return (_cells+coord2indx(col, row))->_type;
	}

	int getTerrain(Vector2D *point)
	{
		if (!isValideCoord(poit->col, point->row))
		{
			return E_WALL;
		}
		return (_cells+coord2indx(point->col, point->row))->_type;
	}
	
	
	int getTerrain(Vector2D &point)
	{
		if (!isValideCoord(poit.col, point.row))
		{
			return E_WALL;
		}
		return (_cells+coord2indx(point.col, point.row))->_type;
	}
	
	Cell * getCell(Vector2D *point)
	{
		if (! isValideCoord(point->col, point-row))
		{
			return NULL;
		}
		return _cells + coord2index(point->col, point->row);

	}
	Cell * getCeoll(int col, int row)
	{
		if (!isValideCoord(col, row))
		{
			return NULL;
		}
		return _cells + coord2index(col, row);
		
	}
	Cell *getCell(int index)
	{
		if (index <0 || index >= _width*_height)
		{
			return NULL;
		}
		return _cells+index;
	}
	
	void setBullet(int col, int row, E_BULLET type)
	{
		if (! isValideCoord(col, row))
		{
			return ;
		}
		(_cells+coord2index(col,row))->_bullet = type;
	}
	
	void dump();
	bool Load(const char * FileName);
	bool Load(std::ifstream & stream);
	
	bool Save(const char * FileName);
	bool Save (std::ofstream & stream);

};
