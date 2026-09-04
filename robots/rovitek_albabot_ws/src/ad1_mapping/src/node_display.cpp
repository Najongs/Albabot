#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include <agv_define.h>
#include <agvmapper.h>

visualization_msgs::Marker GenMark(uint8_t id, double x, double y);
visualization_msgs::Marker GenMarkText(uint8_t id, double x, double y);
visualization_msgs::MarkerArray markArr;

ros::Publisher g_pubMarker;

ros::Time timestamp, last_time;
double dt=0;

void PublishMarker(ros::Time current_time);

AgvMapper *pAgvMapper;


int main(int argc, char **argv)
{
  ros::init(argc, argv, "ad1_mapping");
  ros::NodeHandle nh;

  pAgvMapper=new AgvMapper(&nh);

  g_pubMarker = nh.advertise<visualization_msgs::MarkerArray>("/markers",100);
  pAgvMapper->LoadMap();
  std::list<MAP_INFO>::iterator iter;
  for(iter=pAgvMapper->m_listNodeData.begin();iter!=pAgvMapper->m_listNodeData.end();++iter)
  {
    markArr.markers.push_back(GenMark(iter->id,iter->x,iter->y));
    markArr.markers.push_back(GenMarkText(iter->id,iter->x,iter->y));
  }


  ros::Rate loop_rate(10);

  while(ros::ok())
  {
    timestamp=ros::Time::now();
    PublishMarker(timestamp);

    ros::spinOnce();
    loop_rate.sleep();
  }

  delete pAgvMapper;

  return 1;
}


visualization_msgs::Marker GenMarkText(uint8_t id, double x, double y)
{
  visualization_msgs::Marker text;
  text.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
  text.action = visualization_msgs::Marker::ADD;
  text.header.frame_id = "/odom";
  text.color.r = text.color.g = text.color.b = text.color.a = 1.0f;
  text.id = id+1000;
  text.scale.x = text.scale.y = text.scale.z = 0.1;
  text.pose.position.x = x+0.2;
  text.pose.position.y = y+0.2;
  text.pose.position.z = 0.1;
  text.pose.orientation.x=0;
  text.pose.orientation.y=0.707;
  text.pose.orientation.z=0;
  text.pose.orientation.w=0.707;
  text.ns = "text";
  text.text = std::to_string(id);

  return text;
}

visualization_msgs::Marker GenMark(uint8_t id, double x, double y)
{
  visualization_msgs::Marker marker;
  marker.header.frame_id="/odom";
  marker.header.stamp=ros::Time::now();
  marker.id=pAgvMapper->GetCurID();
  marker.ns="basic_shapes";
  marker.type=visualization_msgs::Marker::CUBE;
  marker.action = visualization_msgs::Marker::ADD;

  marker.pose.position.x = x;
  marker.pose.position.y = y;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;

  marker.scale.x = 0.05;
  marker.scale.y = 0.05;
  marker.scale.z = 0.01;
  marker.color.r = 0.0f;
  marker.color.g = 1.0f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0;


  return marker;
}

void PublishMarker(ros::Time current_time)
{
  g_pubMarker.publish(markArr);
}
