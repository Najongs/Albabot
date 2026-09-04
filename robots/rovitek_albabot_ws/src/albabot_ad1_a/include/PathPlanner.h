#include <bits/stdc++.h>

using namespace std;

//#define ROW 9
//#define COL 10

// Creating a shortcut for int, int pair type
typedef pair<int, int> Pair;

// Creating a shortcut for pair<int, pair<int, int>> type
typedef pair<double, pair<int, int>> pPair;

// A structure to hold the neccesary parameters
struct cell
{
    // Row and Column index of its parent
    // Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
    int parent_i, parent_j;
    // f = g + h
    double f, g, h;
};


class CPathPlanner
{
private:
  int m_nRow;
  int m_nCol;
  bool isValid(int row, int col);
  bool isUnBlocked(int *grid, int row, int col);
  bool isDestination(int row, int col, Pair dest);
  double calculateHValue(int row, int col, Pair dest);
  void tracePath(cell *cellDetails, Pair dest);
  void aStarSearch(int *grid, Pair src, Pair dest);

public:
  string GeneratePathplan(int *_area, pair<int,int> src, pair<int,int> dest, int _dir);
  CPathPlanner();
  CPathPlanner(int row, int col);
  void GetPathPair(pair<int,int> *pPath);
  int GetPathSize();
  pair<int,int> *m_pPath;
  int m_nPathNum;
};

