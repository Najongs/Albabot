#include "PathPlanner.h"

using namespace std;


CPathPlanner::CPathPlanner()
{
  m_nRow=9;
  m_nCol=10;

  m_pPath=nullptr;
}

CPathPlanner::CPathPlanner(int row, int col)
{
  m_nRow=row;
  m_nCol=col;
}

// A Utility Function to check whether given cell (row, col)
// is a valid cell or not.
bool CPathPlanner::isValid(int row, int col)
{
    // Returns true if row number and column number
    // is in range
    return (row >= 0) && (row < m_nRow) &&
           (col >= 0) && (col < m_nCol);
}

// A Utility Function to check whether the given cell is
// blocked or not
bool CPathPlanner::isUnBlocked(int *grid, int row, int col)
{

    // Returns true if the cell is not blocked else false
    if (grid[row*m_nCol+col] == 1)
        return (true);
    else
        return (false);
}

// A Utility Function to check whether destination cell has
// been reached or not
bool CPathPlanner::isDestination(int row, int col, Pair dest)
{
    if (row == dest.first && col == dest.second)
        return (true);
    else
        return (false);
}

// A Utility Function to calculate the 'h' heuristics.
double CPathPlanner::calculateHValue(int row, int col, Pair dest)
{
    // Return using the distance formula
    return ((double)sqrt ((row-dest.first)*(row-dest.first)
                          + (col-dest.second)*(col-dest.second)));
}

// A Utility Function to trace the path from the source
// to destination
void CPathPlanner::tracePath(cell *cellDetails, Pair dest)
{
    printf ("\nThe Path is ");
    int row = dest.first;
    int col = dest.second;

    stack<Pair> Path;

    int index=row*m_nCol+col;

    while (!(cellDetails[index].parent_i == row
             && cellDetails[index].parent_j == col ))
    {

        Path.push (make_pair (row, col));
        int temp_row = cellDetails[index].parent_i;
        int temp_col = cellDetails[index].parent_j;
        row = temp_row;
        col = temp_col;

        index=row*m_nCol+col;
    }    
    Path.push (make_pair (row, col));

    int idx=0;
    int nPathSize=Path.size();

    //pair<int,int> *pPath;

    if(m_pPath!=nullptr)
    {
      delete[] m_pPath;
    }
    m_pPath=new pair<int,int>[Path.size()];
    m_nPathNum=nPathSize;


    while (!Path.empty())
    {
        pair<int,int> p = Path.top();
        Path.pop();
        m_pPath[idx++]=p;
        //printf("-> (%d,%d) ",p.first,p.second);
    }

    for(int i=0;i<m_nPathNum;i++)
    {
      //printf("-> (%d,%d) ",m_pPath[i].first,m_pPath[i].second);
        printf("-> (%d,%d) ",m_pPath[i].second,m_pPath[i].first);
    }


    return;
}
int CPathPlanner::GetPathSize()
{
    return m_nPathNum;
}

void CPathPlanner::GetPathPair(pair<int,int> *pPath)
{
    pPath=m_pPath;
}

string CPathPlanner::GeneratePathplan(int *_area, pair<int,int> src, pair<int,int> dest, int _dir)
{
  string strTemp = "";
  string strCommand = "";
  int *pGrid = new int[m_nRow*m_nCol];
  for(int y=0;y<m_nRow;y++)
  {
    for(int x=0;x<m_nCol;x++)
    {
      if(_area[y*m_nCol+x]>0)
      {
        pGrid[y*m_nCol+x]=1;
      }
      else
      {
        pGrid[y*m_nCol+x]=0;
      }
    }
  }
  aStarSearch(pGrid, src, dest);

  int num=m_nPathNum;
  pair<int,int> *_path=m_pPath;

  int nextDir = 1;
  int dir = _dir;

  for (int i = 0; i < num; i++)
  {

      if (i < num - 1)
      {
          if (_path[i + 1].second - _path[i].second == 1)
              nextDir = 2;
          else if (_path[i + 1].second - _path[i].second == -1)
              nextDir = 4;
          else if (_path[i + 1].first - _path[i].first == 1)
              nextDir = 3;
          else if (_path[i + 1].first - _path[i].first == -1)
              nextDir = 1;

          // 0 : NO way, 1 : load, 2 : node, 3 : terminal
          if (_area[_path[i].first*m_nCol+_path[i].second] != 0 && _area[_path[i].first*m_nCol+_path[i].second] != 1)
          {
              if (dir == nextDir)
              {
                  strCommand += "1";
              }
              else if (abs(dir - nextDir) == 2)
              {
                  strCommand += "4";
                  strCommand += "6";
                  strCommand += "4";
                  strCommand += "1";
              }
              else if (dir - nextDir == -1 || dir - nextDir == 3)
              {
                  strCommand += "4";
                  strCommand += "3";
                  strCommand += "4";
                  strCommand += "1";
              }
              else if (dir - nextDir == 1 || dir - nextDir == -3)
              {
                  strCommand += "4";
                  strCommand += "2";
                  strCommand += "4";
                  strCommand += "1";
              }
          }
          //Console.WriteLine("CurPos [{0},{1}], Target[{2},{3}], Dir {4}, Next {5}\n", _startTile.X, _startTile.Y,
          //    _targetTile.X, _targetTile.Y, dir, nextDir);
          dir = nextDir;
      }
  }

  delete[] pGrid;


  printf("strCommand : %s",strCommand.c_str());
  return strCommand;
}

// A Function to find the shortest path between
// a given source cell to a destination cell according
// to A* Search Algorithm
void CPathPlanner::aStarSearch(int *grid, Pair src, Pair dest)
{
    // If the source is out of range
    if (isValid (src.first, src.second) == false)
    {
        printf ("Source is invalid\n");
        return;
    }

    // If the destination is out of range
    if (isValid (dest.first, dest.second) == false)
    {
        printf ("Destination is invalid\n");
        return;
    }

    // Either the source or the destination is blocked
    if (isUnBlocked(grid, src.first, src.second) == false ||
            isUnBlocked(grid, dest.first, dest.second) == false)
    {
        printf ("Source or the destination is blocked\n");
        return;
    }

    // If the destination cell is the same as source cell
    if (isDestination(src.first, src.second, dest) == true)
    {
        printf ("We are already at the destination\n");
        return;
    }

    // Create a closed list and initialise it to false which means
    // that no cell has been included yet
    // This closed list is implemented as a boolean 2D array
    bool *closedList=new bool[m_nRow*m_nCol];
    memset(closedList, false, sizeof(bool)*m_nRow*m_nCol);

    // Declare a 2D array of structure to hold the details
    //of that cell
    cell *cellDetails=new cell[m_nRow*m_nCol];



    int i, j;
    int index;

    for (i=0; i<m_nRow; i++)
    {
        for (j=0; j<m_nCol; j++)
        {
          index=i*m_nCol+j;

            cellDetails[index].f = FLT_MAX;
            cellDetails[index].g = FLT_MAX;
            cellDetails[index].h = FLT_MAX;
            cellDetails[index].parent_i = -1;
            cellDetails[index].parent_j = -1;
        }
    }

    // Initialising the parameters of the starting node
    i = src.first, j = src.second;
    index=i*m_nCol+j;
    cellDetails[index].f = 0.0;
    cellDetails[index].g = 0.0;
    cellDetails[index].h = 0.0;
    cellDetails[index].parent_i = i;
    cellDetails[index].parent_j = j;

    /*
     Create an open list having information as-
     <f, <i, j>>
     where f = g + h,
     and i, j are the row and column index of that cell
     Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
     This open list is implenented as a set of pair of pair.*/
    set<pPair> openList;

    // Put the starting cell on the open list and set its
    // 'f' as 0
    openList.insert(make_pair (0.0, make_pair (i, j)));

    // We set this boolean value as false as initially
    // the destination is not reached.
    bool foundDest = false;

    while (!openList.empty())
    {
        pPair p = *openList.begin();

        // Remove this vertex from the open list
        openList.erase(openList.begin());

        // Add this vertex to the closed list
        i = p.second.first;
        j = p.second.second;
        index=i*m_nCol+j;
        closedList[index] = true;

        printf ("p: %d, %d\n", i,j);

       /*
        Generating all the 8 successor of this cell

            N.W   N   N.E
              \   |   /
               \  |  /
            W----Cell----E
                 / | \
               /   |  \
            S.W    S   S.E

        Cell-->Popped Cell (i, j)
        N -->  North       (i-1, j)
        S -->  South       (i+1, j)
        E -->  East        (i, j+1)
        W -->  West           (i, j-1)
        N.E--> North-East  (i-1, j+1)
        N.W--> North-West  (i-1, j-1)
        S.E--> South-East  (i+1, j+1)
        S.W--> South-West  (i+1, j-1)*/

        // To store the 'g', 'h' and 'f' of the 8 successors
        double gNew, hNew, fNew;

        //----------- 1st Successor (North) ------------

        // Only process this cell if this is a valid one
        if (isValid(i-1, j) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i-1, j, dest) == true)
            {

                // Set the Parent of the destination cell
                cellDetails[(i-1)*m_nCol+j].parent_i = i;
                cellDetails[(i-1)*m_nCol+j].parent_j = j;
                printf ("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }
            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[(i-1)*m_nCol+j] == false &&
                     isUnBlocked(grid, i-1, j) == true)
            {
                gNew = cellDetails[(i)*m_nCol+j].g + 1.0;
                hNew = calculateHValue (i-1, j, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[(i-1)*m_nCol+j].f == FLT_MAX ||
                        cellDetails[(i-1)*m_nCol+j].f > fNew)
                {
                    openList.insert( make_pair(fNew,
                                               make_pair(i-1, j)));

                    // Update the details of this cell
                    cellDetails[(i-1)*m_nCol+j].f = fNew;
                    cellDetails[(i-1)*m_nCol+j].g = gNew;
                    cellDetails[(i-1)*m_nCol+j].h = hNew;
                    cellDetails[(i-1)*m_nCol+j].parent_i = i;
                    cellDetails[(i-1)*m_nCol+j].parent_j = j;
                }
            }
        }

        //----------- 2nd Successor (South) ------------

        // Only process this cell if this is a valid one
        if (isValid(i+1, j) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i+1, j, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[(i+1)*m_nCol+j].parent_i = i;
                cellDetails[(i+1)*m_nCol+j].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }
            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[(i+1)*m_nCol+j] == false &&
                     isUnBlocked(grid, i+1, j) == true)
            {
                gNew = cellDetails[(i)*m_nCol+j].g + 1.0;
                hNew = calculateHValue(i+1, j, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[(i+1)*m_nCol+j].f == FLT_MAX ||
                        cellDetails[(i+1)*m_nCol+j].f > fNew)
                {
                    openList.insert( make_pair (fNew, make_pair (i+1, j)));
                    // Update the details of this cell
                    cellDetails[(i+1)*m_nCol+j].f = fNew;
                    cellDetails[(i+1)*m_nCol+j].g = gNew;
                    cellDetails[(i+1)*m_nCol+j].h = hNew;
                    cellDetails[(i+1)*m_nCol+j].parent_i = i;
                    cellDetails[(i+1)*m_nCol+j].parent_j = j;
                }
            }
        }

        //----------- 3rd Successor (East) ------------

        // Only process this cell if this is a valid one
        if (isValid (i, j+1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i, j+1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[(i)*m_nCol+(j+1)].parent_i = i;
                cellDetails[(i)*m_nCol+(j+1)].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[(i)*m_nCol+(j+1)] == false &&
                     isUnBlocked (grid, i, j+1) == true)
            {
                gNew = cellDetails[(i)*m_nCol+(j)].g + 1.0;
                hNew = calculateHValue (i, j+1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[(i)*m_nCol+(j+1)].f == FLT_MAX ||
                        cellDetails[(i)*m_nCol+(j+1)].f > fNew)
                {
                    openList.insert( make_pair(fNew,
                                        make_pair (i, j+1)));

                    // Update the details of this cell
                    cellDetails[(i)*m_nCol+(j+1)].f = fNew;
                    cellDetails[(i)*m_nCol+(j+1)].g = gNew;
                    cellDetails[(i)*m_nCol+(j+1)].h = hNew;
                    cellDetails[(i)*m_nCol+(j+1)].parent_i = i;
                    cellDetails[(i)*m_nCol+(j+1)].parent_j = j;
                }
            }
        }

        //----------- 4th Successor (West) ------------

        // Only process this cell if this is a valid one
        if (isValid(i, j-1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i, j-1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[(i)*m_nCol+(j-1)].parent_i = i;
                cellDetails[(i)*m_nCol+(j-1)].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[(i)*m_nCol+(j-1)] == false &&
                     isUnBlocked(grid, i, j-1) == true)
            {
                gNew = cellDetails[(i)*m_nCol+(j)].g + 1.0;
                hNew = calculateHValue(i, j-1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[(i)*m_nCol+(j-1)].f == FLT_MAX ||
                        cellDetails[(i)*m_nCol+(j-1)].f > fNew)
                {
                    openList.insert( make_pair (fNew,
                                          make_pair (i, j-1)));

                    // Update the details of this cell
                    cellDetails[(i)*m_nCol+(j-1)].f = fNew;
                    cellDetails[(i)*m_nCol+(j-1)].g = gNew;
                    cellDetails[(i)*m_nCol+(j-1)].h = hNew;
                    cellDetails[(i)*m_nCol+(j-1)].parent_i = i;
                    cellDetails[(i)*m_nCol+(j-1)].parent_j = j;
                }
            }
        }
#if 0
        //----------- 5th Successor (North-East) ------------

        // Only process this cell if this is a valid one
        if (isValid(i-1, j+1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i-1, j+1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[i-1][j+1].parent_i = i;
                cellDetails[i-1][j+1].parent_j = j;
                printf ("The destination cell is found\n");
                tracePath (cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i-1][j+1] == false &&
                     isUnBlocked(grid, i-1, j+1) == true)
            {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i-1, j+1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[i-1][j+1].f == FLT_MAX ||
                        cellDetails[i-1][j+1].f > fNew)
                {
                    openList.insert( make_pair (fNew,
                                    make_pair(i-1, j+1)));

                    // Update the details of this cell
                    cellDetails[i-1][j+1].f = fNew;
                    cellDetails[i-1][j+1].g = gNew;
                    cellDetails[i-1][j+1].h = hNew;
                    cellDetails[i-1][j+1].parent_i = i;
                    cellDetails[i-1][j+1].parent_j = j;
                }
            }
        }

        //----------- 6th Successor (North-West) ------------

        // Only process this cell if this is a valid one
        if (isValid (i-1, j-1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination (i-1, j-1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[i-1][j-1].parent_i = i;
                cellDetails[i-1][j-1].parent_j = j;
                printf ("The destination cell is found\n");
                tracePath (cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i-1][j-1] == false &&
                     isUnBlocked(grid, i-1, j-1) == true)
            {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i-1, j-1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[i-1][j-1].f == FLT_MAX ||
                        cellDetails[i-1][j-1].f > fNew)
                {
                    openList.insert( make_pair (fNew, make_pair (i-1, j-1)));
                    // Update the details of this cell
                    cellDetails[i-1][j-1].f = fNew;
                    cellDetails[i-1][j-1].g = gNew;
                    cellDetails[i-1][j-1].h = hNew;
                    cellDetails[i-1][j-1].parent_i = i;
                    cellDetails[i-1][j-1].parent_j = j;
                }
            }
        }

        //----------- 7th Successor (South-East) ------------

        // Only process this cell if this is a valid one
        if (isValid(i+1, j+1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i+1, j+1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[i+1][j+1].parent_i = i;
                cellDetails[i+1][j+1].parent_j = j;
                printf ("The destination cell is found\n");
                tracePath (cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i+1][j+1] == false &&
                     isUnBlocked(grid, i+1, j+1) == true)
            {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i+1, j+1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[i+1][j+1].f == FLT_MAX ||
                        cellDetails[i+1][j+1].f > fNew)
                {
                    openList.insert(make_pair(fNew,
                                        make_pair (i+1, j+1)));

                    // Update the details of this cell
                    cellDetails[i+1][j+1].f = fNew;
                    cellDetails[i+1][j+1].g = gNew;
                    cellDetails[i+1][j+1].h = hNew;
                    cellDetails[i+1][j+1].parent_i = i;
                    cellDetails[i+1][j+1].parent_j = j;
                }
            }
        }

        //----------- 8th Successor (South-West) ------------

        // Only process this cell if this is a valid one
        if (isValid (i+1, j-1) == true)
        {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i+1, j-1, dest) == true)
            {
                // Set the Parent of the destination cell
                cellDetails[i+1][j-1].parent_i = i;
                cellDetails[i+1][j-1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i+1][j-1] == false &&
                     isUnBlocked(grid, i+1, j-1) == true)
            {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i+1, j-1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is better,
                // using 'f' cost as the measure.
                if (cellDetails[i+1][j-1].f == FLT_MAX ||
                        cellDetails[i+1][j-1].f > fNew)
                {
                    openList.insert(make_pair(fNew,
                                        make_pair(i+1, j-1)));

                    // Update the details of this cell
                    cellDetails[i+1][j-1].f = fNew;
                    cellDetails[i+1][j-1].g = gNew;
                    cellDetails[i+1][j-1].h = hNew;
                    cellDetails[i+1][j-1].parent_i = i;
                    cellDetails[i+1][j-1].parent_j = j;
                }
            }
        }
#endif

    }

    delete[] cellDetails;
    delete[] closedList;

    // When the destination cell is not found and the open
    // list is empty, then we conclude that we failed to
    // reach the destiantion cell. This may happen when the
    // there is no way to destination cell (due to blockages)
    if (foundDest == false)
        printf("Failed to find the Destination Cell\n");

    return;
}



