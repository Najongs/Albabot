#ifndef MAPUTILITY_H
#define MAPUTILITY_H

typedef struct _MAP_INFO
{
  int id;
  double x;
  double y;
  int link;
  char strInfo[64];
}MAP_INFO;

class MapUtility
{
private:
  int m_nMapID;
  double m_dbCurPosX;
  double m_dbCurPosY;

  string m_strFileName;
  double

public:
  MapUtility();
  MapUtility(string strFileName);


};

#endif // MAPUTILITY_H
