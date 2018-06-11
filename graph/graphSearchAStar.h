#pragma once
#pragma warning (disable:4786)

#include <vector>
#include <list>
#include <queue>
#include <stack>

#include "denseGraph.h"
#include "direction.h"
#include "indexPriorityQ.h"

#include "aStarPolicies.h"


template <class graph_type, class heuristic>
class Graph_SearchAStar
{

private:

    const graph_type&              m_Graph;

    //indexed into my node. Contains the 'real' accumulative cost to that node
    std::vector<double>             m_GCosts;

    //indexed into by node. Contains the cost from adding m_GCosts[n] to
    //the heuristic cost from n to the target node. This is the vector the
    //iPQ indexes into.
    std::vector<double>             m_FCosts;

    std::vector<int>       m_ShortestPathTree;
    std::vector<int>       m_SearchFrontier;

    int                            m_iSource;
    int                            m_iTarget;

    //the A* search algorithm
    void Search();

public:

    Graph_SearchAStar(graph_type &graph,
                      int   source,
                      int   target):m_Graph(graph),
                                    m_ShortestPathTree(graph.NumNodes(), INVALIDE_NID),
                                    m_SearchFrontier(graph.NumNodes(),INVALIDE_NID),
                                    m_GCosts(graph.NumNodes(), INVALIDE_COST),
                                    m_FCosts(graph.NumNodes(), INVALIDE_COST),
                                    m_iSource(source),
                                    m_iTarget(target)
    {
        Search();
    }

    //returns the vector of edges that the algorithm has examined
    std::vector<const int> GetSPT()const{return m_ShortestPathTree;}

    std::list<list> GetPathToPos(int pos, const char * posName = "position") const;

    //returns a vector of node indexes that comprise the shortest path
    //from the source to the target
    std::list<int> GetPathToTarget()const {return GetPathToPos(m_iTarget, "target")};

    int GetCostToPos(int pos, const char * posName = "position") const;
    //returns the total cost to the target
    double GetCostToTarget()const{return GetCostToPos(m_iTarget);}
};

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
void Graph_SearchAStar<graph_type, heuristic>::Search()
{
    //create an indexed priority queue of nodes. The nodes with the
    //lowest overall F cost (G+H) are positioned at the front.
    IndexedPriorityQLow<int> pq(m_FCosts, m_Graph.NumNodes());

    //put the source node on the queue
    pq.push(m_iSource);
    m_GCosts[m_iSource] = 0;

    //while the queue is not empty
    while(!pq.empty())
    {
        //get lowest cost node from the queue
        int NextClosestNode = pq.Pop();

        //move this node from the frontier to the spanning tree
        m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

        //if the target has been found exit
        if (NextClosestNode == m_iTarget) return;

        //now to test all the edges attached to this node
        Direction nbDire = getNeighbourDirection(DIRECTRION_NUM);

        for (int i=0; i<DIRECTION_NUM; i++)
        {
           int neighbour = m_Graph.GetNodeNbId(NextClosestNode, nbDire);
           if (INVALIDE_NID == neighbour || neighbour == m_iSource)
           {
               continue;
           }

           if (neighbour != m_iTarget && !m_Graph.isNodeReachable(neighbour))
           {
               continue;
           }
           int HCost = heuristic::Calculate(m_Graph, m_iTarget, neighbour);
           int GCost = m_GCosts[NextClosestNode] + heuristic::Calculate(m_Graph, NextClosestNode, neighbour);
           if (INVALIDE_NID == m_SearchFrontier[neighbour])
           {
               m_FCosts[neighbour] = GCost + HCost;
               m_GCosts[neighbour] = GCost;
               pq.push(neighbour);
               m_SearchFrontier[neighbour] = NextClosestNode;
           }
           else if ((GCost <m_GCosts[neighbour]) && (INVALIDE_NID) == m_ShortestPathTree[neighbour])
           {
               m_FCosts[neighbour] = GCost + HCost;
               m_GCosts[neighbour] = GCost;
               pq.ChangePriority(neighbour);
               m_SearchFrontier[neighbour] = NextClosestNode;
           }
        }
    }
}

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
std::list<int> Graph_SearchAStar<graph_type, heuristic>::GetPathToTarget()const
{
    std::list<int> path;

    //just return an empty path if no target or no path found
    if (m_iTarget < 0 && pos != m_iTarget)  return path;

    if (pos < 0|| pos >= m_Graph.NumNodes()) return path;

    if (m_ShortestPathTree[pos] == INVALIDE_NID) return path;

    int nd = pos;

    path.push_front(nd);

    while ((nd != m_iSource) && (m_ShortestPathTree[nd] != INVALIDE_NID))
    {
        nd = m_ShortestPathTree[nd];

        path.push_front(nd);
    }

    return path;
}


template <class graph_type, class heuristic>
int Graph_SearchAStar<graph_type, heuristic>::GetCostToPos(int pos, const char * posName = "position") const
{
    if (m_iTarget >= 0 && pos != m_iTarget) {
        return INVALIDE_COST;
    }

    if (pos < 0 || pos >= m_Graph.NumNodes())
    {
        return INVALIDE_COST;
    }
    if (INVALIDE_NID == M_ShortestPathTree[pos] && pos != m_iSource)
    {
        return INVALIDE_CONST;
    }
    return m_GCOSTS[pos];
}
