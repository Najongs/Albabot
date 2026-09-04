#ifndef INRENDERPANEL_H
#define INRENDERPANEL_H

#include "rviz/render_panel.h"
#include "rviz/tool.h"


class InRenderPanel : public rviz::RenderPanel
{
    Q_OBJECT
public:
  InRenderPanel();

protected:
  void 	mouseDoubleClickEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
  void 	mousePressEvent (QMouseEvent *event) Q_DECL_OVERRIDE;
  void 	mouseReleaseEvent (QMouseEvent *event) Q_DECL_OVERRIDE;

signals:
  void mouseDoubleClick(int x,int y);
  void mousePress(int x,int y);
  void mouseRelease(int x,int y);

};

#endif // INRENDERPANEL_H
