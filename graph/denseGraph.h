#pragma once
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//    Name:     SparseGraph.h
//
//    Desc:     Graph class using the adjacency list representation.
//
//    Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <list>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>


#include "grid.h"
#include "direction.h"

template <class NodeType>     
class DenseGraph                                                                 
{
private:

    //the nodes that comprise this graph
    std::vector<NodeType> m_Nodes;
    int m_nRow;
    int m_nCol;
    
public:
    
    void Init(int nRow, int nCol);
    //constrctor
    DenseGraph() {}
    DenseGraph(int nRow, int nCol) { Init(nRow, nCol);}

    //non const version
    const NodeType& GetNode(int idx) const;
    void SetNode(int idx, NodeType &node) { m_Nodes[idx] = node;}
    //adds a node to the graph and returns its index
    void AddNode(int nid, NodeType& node);
    
    bool isNodeValid(int row, int col) const;

    virtual bool isNodeReachable(int id) const;

    int GetIndexByCoord(int row, int col) const { return row * m_nCol + col;}

    const NodeType getNodeByCoord(int row, int col) const { return m_Nodes[GetIndexByCoord(row, col)];}

    void GetCoordByNodeId(int nid, int &row, int &col) const { row = nid / m_nCol; col = nid % m_nCol;}
    Grid GetPos(int nid) const { Grid g; GetCoordByNodeId(nid, g.row, g.col); return g;}
    
    int GetNodeNbId (int nid, Direction& dire) const;

    int NumNodes() const { return m_nRow * m_nCol;}

    //methods for loading and saving graphs from an open file stream or from
    //a file name 
    bool Save(const char* FileName) const;
    bool Save(std::ofstream& stream) const;

    bool SavePathToTarget(const char* FileName, std::list<int> &path) const;
    bool SavePathToTarget(std::ofstream& stream, std::list<int> &path) const;

    bool Load(const char* FileName);
    bool Load(std::ifstream& stream);
    void LoadFromMatrix(NodeType *pMatrix, int nRow, int nCol);

    //clears the graph ready for new node insertions
    void Clear() { m_Nodes.clear();}
};

//------------------------------ GetNode -------------------------------------
//
//    const and non const methods for obtaining a reference to a specific node
//----------------------------------------------------------------------------
template <class NodeType>
const NodeType& DenseGraph<NodeType>::GetNode(int idx) const
{
        assert( (idx < (int)m_Nodes.size()) 
                &&      (idx >=0)
                &&   "<SparseGraph::GetNode>: invalid index");

        return m_Nodes[idx];
}

//----------------------------- GetNodeNbId ----------------------------------
//
//    const and non const methods for obtaining a reference to a specific node
//----------------------------------------------------------------------------
template <class NodeType>
int DenseGraph<NodeType>::GetNodeNbId(int nid, Direction &dire) const
{
    int row, col;

    GetCoordByNodeId(nid, row, col);

    int nbRow = row + dire.dRow;
    int nbCol = col + dire.dCol;

    if (!isNodeValid(nbRow, nbCol))
    {
        return INVALID_NID;
    }

    return GetIndexByCoord(nbRow, nbCol);
}

//-------------------------- AddNode -------------------------------------
//------------------------------------------------------------------------
template <class NodeType>
void DenseGraph<NodeType>::AddNode(int nid, NodeType& node)
{
    if ((size_t)nid >= m_Nodes.size())
    {
        log("<DenseGraph::AddNode>:invalid index(node %d, max %d).", nid, m_Nodes.size() - 1);
        return ;
    }

    m_Nodes[nid] = node;
    return ;
}

//-------------------------------- Save ---------------------------------------

template <class NodeType>
bool DenseGraph<NodeType>::Save(const char* FileName) const
{
    //open the file and make sure it's valid
    std::ofstream out(FileName);

    if (!out)
    {
        throw std::runtime_error("Cannot open file: " + std::string(FileName));
        return false;
    }

    return Save(out);
}

//-------------------------------- Save ---------------------------------------
template <class NodeType>
bool DenseGraph<NodeType>::Save(std::ofstream& stream) const
{
    //save the number of nodes
    stream << m_nRow << ", " << m_nCol << std::endl;

    //iterate through the graph nodes and save them

    for (int i = 0; i < m_nRow; i++)
    {
        for (int j = 0; j < m_nCol; j++)
        {
            NodeType node = getNodeByCoord(i, j);
            stream << node << " ";
        }
        stream << std::endl;
    }

    return true;
}

template <class NodeType>
bool DenseGraph<NodeType>::SavePathToTarget(const char* FileName, std::list<int> &path) const
{
    std::ofstream out(FileName, std::ios::app);

    if (!out)
    {
        throw std::runtime_error("Cannot open file: " + std::string(FileName));
        return false;
    }

    return SavePathToTarget(out, path);
}

template <class NodeType>
bool DenseGraph<NodeType>::SavePathToTarget(std::ofstream& stream, std::list<int> &path) const
{
    std::vector<int> nodes(m_Nodes);
    
    for (size_t i = 0; i < nodes.size(); i++)
    {
        nodes[i] *= -1;
    }

    for (std::list<int>::iterator it = path.begin(); it != path.end(); it++)
    {
        nodes[*it] = nodes[*it] * -10;
    }

    //save the number of nodes
    stream << m_nRow << ", " << m_nCol << std::endl;

    //iterate through the graph nodes and save them

    for (int i = 0; i < m_nRow; i++)
    {
        for (int j = 0; j < m_nCol; j++)
        {
            int nid = GetIndexByCoord(i, j);
            stream << nodes[nid] << " ";
        }
        stream << std::endl;
    }

    return true;
}

//------------------------------- Load ----------------------------------------
//-----------------------------------------------------------------------------
template <class NodeType>
bool DenseGraph<NodeType>::Load(const char* FileName)
{
    //open file and make sure it's valid
    std::ifstream in(FileName);

    if (!in)
    {
        throw std::runtime_error("Cannot open file: " + std::string(FileName));
        return false;
    }

    return Load(in);
}

//------------------------------- Load ----------------------------------------
//-----------------------------------------------------------------------------
template <class NodeType>
bool DenseGraph<NodeType>::Load(std::ifstream& stream)
{
    Clear();

    //get the number of nodes and read them in
    int nCol, nRow;
    int val;
    NodeType node;

    stream >> nRow;
    stream >> nCol;

    Init(nRow, nCol);

    for (int i = 0; i < m_nRow; i++)
    {
        for (int j = 0; j < m_nCol; j++)
        {
            stream >> node;
            AddNode(GetIndexByCoord(i, j), node);
        }
    }
    return true;
}
//--------------------------------- LoadFromMatrix ----------------------------
//-----------------------------------------------------------------------------
template <class NodeType>
void DenseGraph<NodeType>::LoadFromMatrix( NodeType *pMatrix, int nRow, int nCol)
{
    Init(nRow, nCol);

    int totalNum = nRow * nCol;
    //save the terrain
    for (int t = 0; t < totalNum; ++t)
    {
        AddNode(t, pMatrix[t]);
    }
}

//------------------------------- Init ----------------------------------------
//-----------------------------------------------------------------------------
template <class NodeType>
void DenseGraph<NodeType>::Init(int nRow, int nCol)
{
    m_nRow = nRow;
    m_nCol = nCol;
    m_Nodes.resize(nRow * nCol);
    return ;
}


//--------------------------- isNodeValid --------------------------------
//
//    returns true if a node with the given index is present in the graph
//--------------------------------------------------------------------------
template <class NodeType>
bool DenseGraph<NodeType>::isNodeValid(int row, int col) const
{
    if ((row < 0) || (row >= m_nRow) || (col < 0) || (col >= m_nCol))
    {
        return false;
    }
    return true;
}

//--------------------------- isNodeReachable --------------------------------
//
//    returns true if a node with the given index is present in the graph
//--------------------------------------------------------------------------
template <class NodeType>
bool DenseGraph<NodeType>::isNodeReachable(int nid) const
{
    
    return true; 
}
