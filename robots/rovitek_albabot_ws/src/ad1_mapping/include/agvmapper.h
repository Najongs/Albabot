#ifndef AGVMAPPER_H
#define AGVMAPPER_H

#include <ros/ros.h>
#include <geometry_msgs/Pose2D.h>
#include "albabot_msgs/RobotInfo.h"

#include "agv_define.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <bits/stdc++.h>

typedef std::pair<int,int> IndexPair;
typedef std::pair<double,double> CalPair;

typedef struct _MAP_INFO
{
  uint8_t id;
  double x;
  double y;
  double th;
  uint8_t link;
}MAP_INFO;

class AgvMapper
{
public:
  AgvMapper();
  AgvMapper(ros::NodeHandle* nodehandle);
  void SetCurPose(int idxX, int idxY, uint8_t u8InitMapDir,uint8_t u8InitRobotDir);
  void AddNode();
  void DelNode();
  void ClearNode();
  void LoadMap();
  void SaveMap(std::string strFileName);
  void RobotInfoCB(const albabot_msgs::RobotInfo::ConstPtr& value);
  uint8_t GetCurID();
  geometry_msgs::Pose2D GetPose2D();
  //std::list<std::pair<IndexPair,CalPair>> m_listCalData;
  std::list<MAP_INFO> m_listNodeData;

private:

  uint8_t m_u8CurID;
  uint8_t m_u8CurRobotDir;
  uint8_t m_u8InitMapDir;
  uint8_t m_u8InitRobotDir;

  void CaliPosition();
  ros::NodeHandle nh_;

  int m_nIndexX;
  int m_nIndexY;

  double m_dbPosX;
  double m_dbPosY;
  double m_dbCalPosX;
  double m_dbCalPosY;

  double m_dbDistance;
  double m_dbAngle;

  int64_t m_i64PreEncL;
  int64_t m_i64PreEncR;

  uint8_t m_u8Opmode;

  uint8_t m_u8PreAgvStatus;
  uint8_t m_u8PreMission;
  uint32_t m_u32PreMissionCount;


};

#endif // AGVMAPPER_H
