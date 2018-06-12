#include "matrix_map.h"
#include <cassert>
#include "constants.h"
#include <iomanip>
#include "debug.h"

using namespace std;

bool MatrixMap::m_alloc(int col, int row) {
    _starVec.clear();
    _diamondVec.clear();
    m_free();
    _cells = new Cell[col*row];

    assert(col <= MaxMapWidth);
    assert(row <= MaxMapHeight);
    _width = col;
    _height = row;

    for (int i=0; i<co*row; i++)
    {
        (_cells+i)->_point.Set(index2col(i), index2row(i));
    }
    return true;
}


void MatrixMap::clearVolatileTerrain() {
    for(int i=0 ; i<_height*_width; i++)
    {
        (_cells+i)->_bullet = NO_BULLET;
        if ((_cells+i)->_type == E_FLAT || (_cells+i)->_type == E_RIVER)
        {
            continue;
        }
        (_cells+i)->_type = E_FLAT;
    }
}


void MatrixMap::clearVolatileTerrain(E_CELL type) {
    for(int i=0 ; i<_height*_width; i++)
    {
        if ((_cells+i)->_type == type)
        {
            (_cells+i)->_type = E_FLAT;
        }
    }
}

void MatrixMap::clearDeletedTerrain() {
    for(int i=0 ; i<_height*_width; i++)
    {
        if ((_cells+i)->_deleted == true)
        {
            (_cells+i)->_type = E_FLAT;
            (_cells+i)->_deleted = false;
        }
    }
}

void MatrixMap::clone(MatrixMap &newMap) {
    newMap.m_free();
    newMap._width = _width;
    newMap._height = _height;
    newMap._cells = new Cell[_width*_height];
    assert(newMap._cells != NULL);
    memcpy(newMap._cells, _cells, sizeof(Cell)*_width*_height);
    newMap._starVec.assign(_starVec.begin(), _starVec.end());
	newMap._diamondVec.assign(_diamondVec.begin(),_diamondVec.end());
}

int MatrixMap::getTerrainNumInDistance(Vector2D &pos, E_CELL type) {
    int cnt = 0;
    for (int i = pos.col - 2; i<= pos.col +2; i++)
    {
        for (int j = pos.row-2; j<=pos.row+2; j++)
        {
            if (!isValidCoord(i,j)){
                continue;
            }
            if (getTerrain(i,j) == type && (abs(pos.col -i) + abs(pos.row-j) <= 2)){
                cnt++;
            }
        }
    }
    return cnt;

}

int MatrixMap::getNearestTerrainDistance(Vector2D &pos, Vector2D &dir, E_CELL type)
{
    int distance = 0;
    Vector2D tmp = pos;
    tmp += dir;
    Cell * pCell = getCell(&tmp);
    while(NULL != pCell)
    {
        distance++;
        if (pCell->_type == type)
        {
            return distance;
        }
        if (pCell->_type == E_WALL || pCell->_type == E_BRICK)
        {//遇到障碍，不再继续查询
            break;
        }
        tmp += dir;
        pCell = getCell(&tmp);
    }
    return 0;//表示这个方向没有这种地形
}


int MatrixMap::getTerrainNumInVision(Vector2D &pos, E_CELL type) {
    int num = 0;
    num += getTerrainNumInVision(pos,VECTOR_UP, E_FOE);
    num += getTerrainNumInVision(pos, VECTOR_DOWN, E_FOE);
    num += getTerrainNumInVision(pos, VECTOR_LEFT, E_FOE);
    num += getTerrainNumInVision(pos, VECTOR_RIGHT, E_FOE);
    if (getTerrain(pos) == type)
    {
        num += 1;
    }
    return num;
}

int MatrixMap::getPlayerNumInOneStep(Vector2D &pos)
{
    int num =0;
    E_CELL type = E_FLAT;
    type = (E_CELL)getTerrain(pos + VECTOR_UP);
    if (type = E_FOE || type == E_FRIEND)
    {
        num++;
    }
    type = (E_CELL)getTerrain(pos + VECTOR_DOWN);
    if (type = E_FOE || type == E_FRIEND)
    {
        num++;
    }
    type = (E_CELL)getTerrain(pos + VECTOR_LEFT);
    if (type = E_FOE || type == E_FRIEND)
    {
        num++;
    }
    type = (E_CELL)getTerrain(pos + VECTOR_RIGHT);
    if (type = E_FOE || type == E_FRIEND)
    {
        num++;
    }
    type = (E_CELL)getTerrain(pos + VECTOR_ZERO);
    if (type == E_FOE || type == E_FRIEND)
    {
        num++;
    }
    return num ;
}

int MatrixMap::getTerrainNumInVision(Vector2D &pos, Vector2D &dir, E_CELL type)
{
    int dist = 0;
    int num = 0;
    Vector2D tmp = pos;
    tmp += dir; dist++;
    Cell * pCell = getCell(&tmp);
    while (NULL != pCell && dist <= 6)
    {
        if (pCell->_type == type) {
            num++;
        }
        if (pCell->_type == E_WALL || pCell->_type == E_BRICK)
        {
            break;
        }
        tmp+= dir; dist++;
        pCell = getCell(&tmp);
    }
    return num;
}

bool MatrixMap::Load(const char *FileName)
{
    std::ifstream in(FileName);
    if (!in)
    {
        throw  std::runtime_error("Cannot open file: " + std::string(FileName));
		return false;
    }
    return Load(in);
}

bool MatrixMap::Load(std::ifstream &stream) {
    int nRow = 16, nCol = 16;
    int val;
    stream >> nRow >> nCol;
    m_alloc(nCol, nRow);

    for (int i = 0; i<nRow; i++)
    {
        for (int j=0; j <nCol; j++)
        {
            stream >> val;
            for (int e = 0; e < E_DIAMOND; e++)
            {
                if (val & (1 << e)){
                    setTerrain(j,i,(E_CELL)e);
                }
            }
            if (val & (1 << E_DIAMOND))
            {
                _diamondVec.push_back(Vector2D(j,i));
            }
            if (val & (1 << E_STAR))
            {
                _starVec.push_back(Vector2D(j,i));
            }
        }
    }
    return true;
}

bool MatrixMap::Save(const char * FileName)
{
    std::ofstream out(FileName);
    if (!out)
    {
        throw std::runtime_error("Cannot open file: " + std::string(FileName));
		return false;
    }
    return Save(out);
}

bool MatrixMap::Save(std::ofstream &stream) {
    int nRow = _height, nCol = _width;
    int val = 0;
    int *cellMap = new int[nRow*nCol];
    memset(cellMap, 0, sizeof(int)*nRow * nCol);

    stream << setw(8) << nRow;
    stream << setw(8) << nCol << std::endl;

    for (int i = 0; i<nRow; i++)
    {
        for (int j=0; j < nCol; j++)
        {
            cellMap[coord2index(j,i)] = (1 << getTerrain(j,i));
        }
    }

    for (std::vector<Vector2D>::iterator it = _starVec.begin(); it != _starVec.end(); it++)
    {
        if (1 == cellMap[coord2index(it->col, it->row)]){
            cellMap[coord2index(it->col, it->row)] = 0;
        }
        cellMap[coord2index(it->col, it->row)] += (1 << E_STAR);
    }

    for (std::vector<Vector2D>::iterator it = _diamondVec.begin(); it != _diamondVec.end(); it++)
    {
        if (1 == cellMap[coord2index(it->col, it->row)]){
            cellMap[coord2index(it->col, it->row)] = 0;
        }
        cellMap[coord2index(it->col, it->row)] += (1 << E_DIAMOND);
    }

    for(int i=0; i < nRow; i++)
    {
        for (int j=0; j < nCol; j++)
        {
            stream << setw(8) << cellMap[coord2index(j,i)];
        }
        stream << std::endl;
    }
    delete [] cellMap;
    return true;
}

void MatrixMap::dump()
{
    for (int j = 0; j<_height; j++)
    {
        for (int i=0; i<_width; i++)
        {
            log("%d ", getTerrain(i,j));
        }
        log("\n");
    }
}