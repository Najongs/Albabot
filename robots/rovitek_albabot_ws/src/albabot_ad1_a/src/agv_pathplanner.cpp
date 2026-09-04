#include <ros/ros.h>

#include "PathPlanner.h"
#include <std_msgs/String.h>

int *g_pMap=NULL;
int g_nMapSizeX;
int g_nMapSizeY;
int g_nSrcPosY,g_nSrcPosX;
int g_nDestPosY,g_nDestPosX;


/* protocol :
          set map data : 0,mapsizeX,mapsizeY,mapData
          get path plan : 1, startX, startY, endX, endY
*/
void LoadMapCb(const std_msgs::String::ConstPtr& value)
{

    //ROS_INFO("%s, %d", value->data.c_str(), value->data.length());
    //char *pStrLine=value->data.c_str();
    int x,y,cmdIndex;
    char pMap[1024];
    printf("Test : %c\n", value->data.c_str()[0]);
    cmdIndex=value->data.c_str()[0]-'0';
    int numElem;

    if(cmdIndex==0)         // set map
    {
        numElem=sscanf(value->data.c_str(),"%d,%d,%d,%s",&cmdIndex, &g_nMapSizeX,&g_nMapSizeY,pMap);
        ROS_INFO("%d,%d => %s", g_nMapSizeX,g_nMapSizeY,pMap);
        int nMapSize=g_nMapSizeX*g_nMapSizeY;

        if(g_pMap!=NULL)
        {
            //free(g_pMap);
            //g_pMap=(int*)malloc(nMapSize*sizeof(int));

            delete[] g_pMap;
            g_pMap=new int[g_nMapSizeX*g_nMapSizeY];

        }
        else
        {
            g_pMap=new int[g_nMapSizeX*g_nMapSizeY];
        }


        for(int i=0;i<nMapSize;i++)
        {
            if(pMap[i]-'0'<4)
                g_pMap[i]=pMap[i]-'0';

            if(i%(g_nMapSizeX)==g_nMapSizeX-1)
                printf("%d\n",g_pMap[i]);
            else {
                printf("%d",g_pMap[i]);
            }
        }
        printf("\n");


        /*CPathPlanner planner(g_nMapSizeX,g_nMapSizeY);
        Pair src = make_pair(0,4);    // (y,x)
        Pair dest = make_pair(4,0);   // (y,x)
        int curDir=1;
        string strPath=planner.GeneratePathplan(g_pMap,src,dest,curDir);

        printf("\nPathPlan : %s\n", strPath.c_str());*/

    }
    else if(cmdIndex==1)    // gen pathplan
    {
        numElem=sscanf(value->data.c_str(),"%d,%d,%d,%d,%d",&cmdIndex, &g_nSrcPosX,&g_nSrcPosY,&g_nDestPosX,&g_nDestPosY);
        CPathPlanner planner(g_nMapSizeY,g_nMapSizeX);
        //CPathPlanner planner(g_nMapSizeX,g_nMapSizeY);
        Pair src = make_pair(g_nSrcPosY,g_nSrcPosX);    // (y,x)
        Pair dest = make_pair(g_nDestPosY,g_nDestPosX);   // (y,x)
        int curDir=1;
        string strPath=planner.GeneratePathplan(g_pMap,src,dest,curDir);

        printf("\nPathPlan : %s\n", strPath.c_str());
        //pair<int,int> *pPath=NULL;
        //planner.GetPathPair(pPath);

        printf("The Paht is : ");
        for(int i=0;i<planner.m_nPathNum;i++)
        {
          //printf("-> (%d,%d) ",m_pPath[i].first,m_pPath[i].second);
            printf("-> (%d,%d) ",planner.m_pPath[i].second,planner.m_pPath[i].first);
        }
        printf("\n");
    }
}



int main(int argc, char **argv)
{
  ros::init(argc, argv, "agv_pathplanner");
  ros::NodeHandle nh;

  ros::Subscriber subMap=nh.subscribe("agv_map",100,LoadMapCb);
  //int nRow=9;
  //int nCol=10;

  /*int grid[9][10] =
  {
      { 1, 0, 1, 1, 1, 1, 0, 1, 1, 1 },
      { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1 },
      { 1, 1, 1, 0, 1, 1, 0, 1, 0, 1 },
      { 0, 0, 1, 0, 1, 0, 0, 0, 0, 1 },
      { 1, 1, 1, 0, 1, 1, 1, 0, 1, 0 },
      { 1, 0, 1, 1, 1, 1, 0, 1, 0, 0 },
      { 1, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
      { 1, 0, 1, 1, 1, 1, 0, 1, 1, 1 },
      { 1, 1, 1, 0, 0, 0, 1, 0, 0, 1 }
  };*/

  /*int grid[9*10] =
  {
    2, 0, 2, 1, 1, 1, 0, 1, 1, 1,
    1, 0, 1, 0, 1, 1, 1, 0, 1, 1,
    2, 1, 2, 0, 1, 1, 0, 1, 0, 1,
    0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
    2, 1, 2, 0, 1, 1, 1, 0, 1, 0,
    1, 0, 2, 1, 1, 1, 0, 1, 0, 0,
    1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 1, 1, 0, 1, 1, 1,
    2, 1, 1, 0, 0, 0, 1, 0, 0, 1
  };*/

  /*int grid[9*10] =
  {
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0,
    0, 0, 0,0, 0, 0,0, 0, 0, 0
  };*/
  int grid[9*10] =
  {
    0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    3, 1, 1, 1, 3, 1, 1, 1, 1, 3,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 3, 0, 0, 0, 0, 0
  };

  //9,10,000030000000001000000000100000000010000031113111130000100000000010000000001000000000300000
  /*CPathPlanner planner(9,10);
  Pair src = make_pair(0,4);    // (y,x)
  Pair dest = make_pair(4,0);   // (y,x)
  int curDir=1;
  string strPath=planner.GeneratePathplan(grid,src,dest,curDir);

  printf("\nPathPlan : %s\n", strPath.c_str());*/

  ros::spin();


  return 1;

}
