#include "inrenderpanel.h"
#include <QTabWidget>
#include "rviz/viewport_mouse_event.h"
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>
#include <rviz/visualization_manager.h>
#include <rviz/mesh_loader.h>
#include <rviz/geometry.h>
#include <rviz/properties/vector_property.h>
#include <QDebug>

InRenderPanel::InRenderPanel():rviz::RenderPanel ()
{

}

void 	InRenderPanel::mouseDoubleClickEvent (QMouseEvent *event)
{
    rviz::ViewportMouseEvent events;
    //events.viewport->
    rviz::RenderPanel::mouseDoubleClickEvent(event);

    emit mouseDoubleClick(this->mouse_x_,this->mouse_y_);

}
void 	InRenderPanel::mousePressEvent (QMouseEvent *event)
{
    Ogre::Vector3 intersection;
    Ogre::Plane ground_plane( Ogre::Vector3::UNIT_Z, 0.0f );
    rviz::RenderPanel::mousePressEvent(event);
    //rviz::getPointOnPlaneFromWindowXY(event->localPos().x())

 //   qDebug()<<"local pos"<<event->localPos().x()<<","<<event->localPos().y()<<endl;
 //   qDebug()<<"global pos"<<event->globalPos().x()<<","<<event->globalPos().y()<<","<<event->globalPos().rx()<<endl;
 //   qDebug()<<"screen pos"<<event->screenPos().x()<<","<<event->screenPos().y()<<endl;

    emit mousePress(this->mouse_x_,this->mouse_y_);

}
void 	InRenderPanel::mouseReleaseEvent (QMouseEvent *event)
{
    rviz::RenderPanel::mouseReleaseEvent(event);
    emit mouseRelease(this->mouse_x_,this->mouse_y_);

}
