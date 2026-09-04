#include "agvmapper.h"

AgvMapper::AgvMapper()
{
  m_u8Opmode=1;
  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");
  m_i64PreEncL=rxMsg->left_enc;
  m_i64PreEncR=rxMsg->right_enc;

  m_nIndexX=0;
  m_nIndexY=0;
  m_dbPosX=0;
  m_dbPosY=0;
  m_dbAngle=0.0;

  m_u8PreAgvStatus=0;
  m_u8PreMission=AGV_MISSION_WAIT;
  m_u32PreMissionCount=0;

  m_u8CurID=0;

}


AgvMapper::AgvMapper(ros::NodeHandle* nodehandle):nh_(*nodehandle)
{
  m_u8Opmode=1;
  albabot_msgs::RobotInfoConstPtr rxMsg = ros::topic::waitForMessage<albabot_msgs::RobotInfo>("/robot_info");
  m_i64PreEncL=rxMsg->left_enc;
  m_i64PreEncR=rxMsg->right_enc;

  m_nIndexX=0;
  m_nIndexY=0;
  m_dbPosX=0;
  m_dbPosY=0;
  m_dbAngle=0.0;

  m_u8PreAgvStatus=0;
  m_u8PreMission=AGV_MISSION_WAIT;
  m_u32PreMissionCount=0;

  m_u8CurID=0;
}



void AgvMapper::DelNode()
{

  if(m_listNodeData.size()>0)
  {
    std::list<MAP_INFO>::iterator iter;
    for(iter=m_listNodeData.begin();iter!=m_listNodeData.end();++iter)
    {
      if(m_u8CurID==iter->id)
      {
        m_listNodeData.erase(iter);
        break;
      }
    }
  }
}

uint8_t AgvMapper::GetCurID()
{
  return m_u8CurID;
}
void AgvMapper::ClearNode()
{
  m_listNodeData.clear();
  m_u8CurID=0;
}


void AgvMapper::AddNode()
{
  int nOverlap=0;
  std::list<std::pair<IndexPair,CalPair>>::iterator iter;

  if(m_listNodeData.size()==0)
  {
    MAP_INFO data;
    data.id=m_u8CurID;
    data.x=m_nIndexX;
    data.y=m_nIndexY;
    data.th=0.0;
    data.link=0;
    m_listNodeData.push_back(data);
    m_u8CurID=1;
  }
  else
  {

    MAP_INFO data;
    data.id=m_u8CurID;
    data.x=m_nIndexX;
    data.y=m_nIndexY;
    data.th=0.0;
    data.link=0;
    m_listNodeData.push_back(data);
    m_u8CurID++;

  }
}



void AgvMapper::SaveMap(std::string strFileName)
{
  FILE *fp;
  char pStrTemp[1024];
  char pStrDir[1024];
  sprintf(pStrDir,"%s/.ros/map/",getenv("HOME"));
  mkdir(pStrDir,755); // rwxr-xr-x
  sprintf(pStrTemp,"%s/.ros/map/%s.txt",getenv("HOME"),strFileName.c_str());
  fp=fopen(pStrTemp,"w+");

  std::list<MAP_INFO>::iterator iter;
  for(iter=m_listNodeData.begin();iter!=m_listNodeData.end();++iter)
  {
    fprintf(fp,"%d,%lf,%lf,%lf,%d\n",iter->id,iter->x,iter->y,iter->th, iter->link);
  }

  fclose(fp);
}


void AgvMapper::LoadMap()
{
  FILE* fp;
  char pFileName[1024];
  sprintf(pFileName,"%s/.ros/map/map.txt",getenv("HOME"));
  fp=fopen(pFileName,"r");
  //ROS_INFO("file name : %s",pFileName);

  if(fp==NULL)
   {
    //ROS_INFO("bye!");
    return;
  }
  int num=0;
  while(!feof(fp))
  {

    MAP_INFO data;
    char pStrLine[2048];
    char pStrTemp[64];

    if(fgets(pStrLine,2048-1,fp)==NULL)
      break;


    int numElem=sscanf(pStrLine,"%d,%lf,%lf,%lf,%d",
                       &(data.id),&(data.x),&(data.y),&(data.th),&(data.link));
    if(numElem==5)
    {
      m_listNodeData.push_back(data);
      ++num;
    }



    /*std::pair<IndexPair,CalPair> data;
    char pStrLine[2048];
    char pStrTemp[64];

    if(fgets(pStrLine,2048-1,fp)==NULL)
      break;


    int numElem=sscanf(pStrLine,"%d,%d,%lf,%lf",&(data.first.first),&(data.first.second),
                       &(data.second.first),&(data.second.second));


    if(numElem==4)
    {
      m_listCalData.push_back(data);

      //ROS_INFO("calibaration point : [%d,%d]=>%.3lf,%.3lf",(data.first.first),(data.first.second),
               //(data.second.first),(data.second.second));
    }*/

  }
  m_u8CurID=num;
  fclose(fp);
}

void AgvMapper::CaliPosition()
{

  /*std::list<std::pair<IndexPair,CalPair>>::iterator iter;
  for(iter=m_listCalData.begin();iter!=m_listCalData.end();++iter)
  {
    if(m_nIndexX==iter->first.first && m_nIndexY==iter->first.second)
    {
      m_dbPosX=iter->second.first;
      m_dbPosY=iter->second.second;
    }
  }*/
}

geometry_msgs::Pose2D AgvMapper::GetPose2D()
{
  geometry_msgs::Pose2D pose;
  pose.x=m_dbPosX;
  pose.y=m_dbPosY;
  pose.theta=m_dbAngle;
  return pose;
}


void AgvMapper::SetCurPose(int idxX, int idxY, uint8_t u8InitMapDir,uint8_t u8InitRobotDir)
{
  m_u8InitMapDir=u8InitMapDir;
  m_u8InitRobotDir=u8InitRobotDir;
}


void AgvMapper::RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value)
{

  m_u8CurRobotDir=value->agvDirection;


  //int mapOffset=(4+(m_u8InitMapDir-m_u8InitRobotDir))%4;  //map dir(1) - init robot(4) (4+(1-4))%4
  int mapOffset=(4+(m_u8InitMapDir-m_u8InitRobotDir))%4;
  uint8_t u8MapDir=1;

  if((value->agvDirection+mapOffset)<=4)
    u8MapDir=(value->agvDirection+mapOffset);
  else
  {
    u8MapDir=(value->agvDirection+mapOffset)-4;
  }

  static uint8_t preNode=0;
  // check node
  if(m_u8PreAgvStatus==1 && value->agvStatus==0)
  {
    if(u8MapDir==1)
      m_nIndexY++;
    else if(u8MapDir==2)
      m_nIndexX++;
    else if(u8MapDir==3)
      m_nIndexY--;
    else if(u8MapDir==4)
      m_nIndexX--;

    //CaliPosition();

    preNode++;

    //printf("preNode : %d\n",preNode);




  }
  else if(value->agvCurrentMission==AGV_MISSION_NEXT && m_u8PreMission!=AGV_MISSION_NEXT)
  {
    if(u8MapDir==1)
      m_nIndexY++;
    else if(u8MapDir==2)
      m_nIndexX++;
    else if(u8MapDir==3)
      m_nIndexY--;
    else if(u8MapDir==4)
      m_nIndexX--;
  }
  else if(value->agvCurrentMission==AGV_MISSION_NEXT &&
          m_u8PreMission==AGV_MISSION_NEXT &&
          m_u32PreMissionCount != value->agvMissionCount)
  {
    if(u8MapDir==1)
      m_nIndexY++;
    else if(u8MapDir==2)
      m_nIndexX++;
    else if(u8MapDir==3)
      m_nIndexY--;
    else if(u8MapDir==4)
      m_nIndexX--;
  }


  //ROS_INFO("offset[%d-%d],dir[%d]:%d, %d => %.3lf, %.3lf",m_u8InitMapDir,m_u8InitRobotDir,u8MapDir, m_nIndexX, m_nIndexY, m_dbPosX, m_dbPosY);


  m_u8PreAgvStatus=value->agvStatus;
  m_u8PreMission=value->agvCurrentMission;
  m_u32PreMissionCount=value->agvMissionCount;

  m_u8Opmode=0;

  float diff;
  double diff1, diff2;
  if(m_u8Opmode==1)       // agv mode
  {
    if(value->agvCurrentMission==AGV_MISSION_TURN)
    {
      m_dbDistance=0;

      diff1=2.0*0.1*((value->left_enc-m_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-m_i64PreEncR)/(50.0*20))*3.141592;
      m_dbAngle+=((diff2-diff1)/0.328);

    }
    else if(value->agvCurrentMission==AGV_MISSION_LEFT_90)
    {
      m_dbDistance=0;
      double diff1, diff2;
      diff1=2.0*0.1*((value->left_enc-m_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-m_i64PreEncR)/(50.0*20))*3.141592;
      m_dbAngle+=((diff2-diff1)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_RIGHT_90)
    {
      m_dbDistance=0;
      diff1=2.0*0.1*((value->left_enc-m_i64PreEncL)/(50.0*20))*3.141592;
      diff2=2.0*0.1*((value->right_enc-m_i64PreEncR)/(50.0*20))*3.141592;
      m_dbAngle+=((diff2-diff1)/0.328);
    }
    else if(value->agvCurrentMission==AGV_MISSION_NEXT)
    {
      //m_dbAngle=(u8MapDir-2)*(3.141592/2.0);

      diff=((float)(value->left_enc-m_i64PreEncL)+(float)(value->right_enc-m_i64PreEncR))/2.0;
      m_dbDistance+=(((0.2*3.141592)/(50.0*20.0))*diff);

      if(u8MapDir==1)
      {
        m_dbPosY+=(((0.2*3.141592)/(50.0*20.0))*diff);//m_dbDistance;
      }
      else if(u8MapDir==2)
      {
        m_dbPosX+=(((0.2*3.141592)/(50.0*20.0))*diff);//m_dbDistance;
      }
      else if(u8MapDir==3)
      {
        m_dbPosY-=(((0.2*3.141592)/(50.0*20.0))*diff);//m_dbDistance;
      }
      else if(u8MapDir==4)
      {
        m_dbPosX-=(((0.2*3.141592)/(50.0*20.0))*diff);//m_dbDistance;
      }
    }
    else if(value->agvCurrentMission==AGV_MISSION_WAIT)
    {
      m_dbDistance=0;
      m_dbAngle=(u8MapDir-2)*(-3.141592/2.0);
    }
  }
  else if(m_u8Opmode==0)
  {
    diff1=2.0*0.1*((value->left_enc-m_i64PreEncL)/(50.0*20))*3.141592;
    diff2=2.0*0.1*((value->right_enc-m_i64PreEncR)/(50.0*20))*3.141592;
    m_dbAngle+=((diff2-diff1)/0.328);

    diff=((float)(value->left_enc-m_i64PreEncL)+(float)(value->right_enc-m_i64PreEncR))/2.0;
    diff1=(((0.2*3.141592)/(50.0*20.0))*diff);
    m_dbPosX+=(diff1*cos(m_dbAngle));
    m_dbPosY+=(diff1*sin(m_dbAngle));
  }
  //ROS_INFO("Distance : %.3f, %.2f",m_dbDistance,m_fAngle);//(m_fAngle/3.141592)*180.0);

  m_i64PreEncL=value->left_enc;
  m_i64PreEncR=value->right_enc;
}


