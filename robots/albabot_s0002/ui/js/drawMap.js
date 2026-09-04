
var nodeInfo={};
nodeInfo.pt1X;
nodeInfo.pt1Y;
nodeInfo.pt2X;
nodeInfo.pt2Y;
nodeInfo.pt1Name;
nodeInfo.pt2Name;

var drawMap={};

// map
drawMap.map_w; // 1 = 10mm
drawMap.map_h;
drawMap.map_res; // (50px) = 10mm
drawMap.mapData = new Array;

drawMap.viewport_x;
drawMap.viewport_y;
drawMap.viewport_w;
drawMap.viewport_h;

drawMap.canvas_w;
drawMap.canvas_h;


// robot pose
drawMap.robotPosX=10;   // cell 좌표 x
drawMap.robotPosY=10;   // cell 좌표 x
drawMap.robotAngle=180; // 각 top=0 

drawMap.selTool=0;      // blank



drawMap.SetMapFromNode=function(node)
{    
    node.pt1X=0;
    node.pt1Y=0;

    node.pt2X=0;
    node.pt2Y=100;
    

    x=parseInt(node.pt1X/drawMap.map_res);
    y=parseInt(node.pt1Y/drawMap.map_res);
    drawMap.SetMapVal(x,y,2);

    
    x=parseInt(node.pt2X/drawMap.map_res);
    y=parseInt(node.pt2Y/drawMap.map_res);
    drawMap.SetMapVal(x,y,2);


        
}

drawMap.SelTools=function(toolId) 
{
    selTool=toolId;    
}

drawMap.SetMapVal=function(x,y,val)
{    
    if(val>=0 && val<=4)
    {
        drawMap.mapData[y][x]=val;
    }
}


// ptX : 
function SetRobotPose(ptX,ptY,angle) 
{
    //
    drawMap.robotPosX=parseInt(ptX/drawMap.map_res);
    drawMap.robotPosY=parseInt(ptY/drawMap.map_res);

    drawMap.robotAngle=angle;
}

function ClearMap()
{
    for(y=0;y<parseInt(drawMap.map_h);y++)
    {
        for(x=0;x<parseInt(drawMap.map_w);x++)
        {             
            drawMap.mapData[y][x]=0;
        }
    }
}



drawMap.DrawRobotPos=function(posX, posY, angle) 
{
    var ctx=document.getElementById("canvas").getContext('2d');

    var cellPosX=parseInt(drawMap.map_w/2)+posX;///map_res;
    var cellPosY=parseInt(drawMap.map_h/2)-posY;///map_res;
                        
    // 그리드 그리기
    /*ctx.beginPath();
    ctx.rect(parseInt(x)*drawMap.map_res, parseInt(y)*drawMap.map_res,drawMap.map_res,drawMap.map_res);
    ctx.strokeStyle = "rgba(0, 0, 0, 0.5)";
    ctx.stroke();
    ctx.closePath();*/       
    
    // 좌표표시            
    ctx.font = '8px Gulim';
    ctx.textBaseline='bottom';
    ctx.textAlign ='end';
    ctx.fillStyle='white';
    ctx.textAlign ='left';
    ctx.fillText(cellPosX+','+cellPosY, parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res,parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res+1);
    //ctx.textAlign ='left';
    //ctx.fillText(cellPosY, parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res,parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res+10);
    
    // 로봇의 현재위치
    ctx.fillStyle="rgb(128,128,128)";
    ctx.fillRect(parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res+1, 
                    parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res+1,drawMap.map_res-1,drawMap.map_res-1);
    
    // 로봇의 방향
    drawMap.canvas_arrow(ctx,posX,posY,angle,cellPosX-drawMap.viewport_x, cellPosY-drawMap.viewport_y);
    
}

drawMap.canvas_arrow=function(context, ptX, ptY, angle, view_px, view_py) 
{
    var pt1_x, pt1_y;
    var pt2_x, pt2_y;
    var pt3_x, pt3_y;

    pt1_x=0;
    pt1_y=-(drawMap.map_res/2)+2;
    
    pt2_x=-(drawMap.map_res/2)+2;
    pt2_y=drawMap.map_res/2-2;

    pt3_x=(drawMap.map_res/2)-2;
    pt3_y=drawMap.map_res/2-2;
    
    context.save();    
    context.translate(parseInt(view_px)*drawMap.map_res+drawMap.map_res/2, parseInt(view_py)*drawMap.map_res+drawMap.map_res/2);
    context.rotate((Math.PI / 180) * angle);
    context.fillStyle="rgb(0,0,255)";
    context.beginPath();
    context.moveTo(pt1_x, pt1_y);
    context.lineTo(pt2_x, pt2_y);
    context.lineTo(pt3_x, pt3_y);
    context.fill();    
    context.restore();
}


drawMap.DrawCurrentView=function(posX, posY, angle) 
{
    var ctx=document.getElementById("canvas").getContext('2d');

    var cellPosX=posX+parseInt(drawMap.map_w/2);///map_res;
    var cellPosY=-posY+parseInt(drawMap.map_h/2);///map_res;

    //console.log("cell : "+cellPosX + ','+cellPosY);

    if(cellPosX<drawMap.viewport_w/2)
    {
        drawMap.viewport_x=0;
    }
    else if(cellPosX>(drawMap.map_w)-drawMap.viewport_w)
    {
        drawMap.viewport_x=parseInt(drawMap.map_w)-drawMap.viewport_w;
    }
    else
    {
        drawMap.viewport_x=cellPosX-parseInt(drawMap.viewport_w/2);
    }

    if(cellPosY<drawMap.viewport_h/2)
    {        
        drawMap.viewport_y=0;
    }
    else if(cellPosY>drawMap.map_h-drawMap.viewport_h/2)
    {
        drawMap.viewport_y=parseInt(drawMap.map_h)-drawMap.viewport_h;
    }
    else
    {
        drawMap.viewport_y=cellPosY-parseInt(drawMap.viewport_h/2);
    }

    //console.log("viewport : " + drawMap.viewport_x + ','+ drawMap.viewport_y+ ','+ drawMap.viewport_w + ',' + drawMap.viewport_h);
    //console.log("cellpos : " + cellPosX + ','+ cellPosY + ',' + drawMap.robotPosX + ','+ drawMap.robotPosY);
    
    // 화면 초기화
    ctx.clearRect(0,0,drawMap.viewport_w*drawMap.map_res,drawMap.viewport_h*drawMap.map_res);   
        
    // 그리드, 맵 컴포넌트 그리기
    for(y=0;y<parseInt(drawMap.viewport_h);y++)
    {
        for(x=0;x<parseInt(drawMap.viewport_w);x++)
        { 
            var idxX,idxY;
            idxX=drawMap.viewport_x+x;
            idxY=drawMap.viewport_y+y;

            // 컴포넌트 그리기
            if(drawMap.mapData[idxY][idxX]==0)    // blank
            {
                ctx.fillStyle="rgb(0,0,0)";
            }
            else if(drawMap.mapData[idxY][idxX]==1)    // way
            {
                ctx.fillStyle="rgb(255,255,255)";
            }
            else if(drawMap.mapData[idxY][idxX]==2)    // node
            {
                ctx.fillStyle="rgb(255,0,0)";
            }
            else if(drawMap.mapData[idxY][idxX]==3)    // terminal / table
            {
                ctx.fillStyle="rgb(0,0,255)";
            }
            ctx.fillRect(parseInt(x)*drawMap.map_res+1, parseInt(y)*drawMap.map_res+1,drawMap.map_res-1,drawMap.map_res-1);
                        
            // 그리드 그리기
            ctx.beginPath();
            ctx.rect(parseInt(x)*drawMap.map_res, parseInt(y)*drawMap.map_res,drawMap.map_res,drawMap.map_res);
            ctx.strokeStyle = "rgba(0, 0, 0, 0.5)";
            ctx.stroke();
            ctx.closePath();       
            
            // 좌표표시            
            ctx.font = '8px Gulim';
            ctx.textBaseline='bottom';
            ctx.textAlign ='end';
            ctx.fillStyle='white';
            //ctx.fillText(idxX, parseInt(x)*drawMap.map_res+1, parseInt(y)*drawMap.map_res);
            ctx.textAlign ='left';
            ctx.fillText(idxX +','+idxY, parseInt(x)*drawMap.map_res+1, parseInt(y)*drawMap.map_res+10);
            
        }
    }


    // 로봇의 현재위치
    ctx.fillStyle="rgb(128,128,128)";
    ctx.fillRect(parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res+1, 
                    parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res+1,drawMap.map_res-1,drawMap.map_res-1);

    //console.log("viewport : "+drawMap.viewport_x+','+drawMap.viewport_y);
    //console.log("pt : "+parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res + 
    //                    ','+parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res);
    // 로봇의 방향
    drawMap.canvas_arrow(ctx,posX,posY,angle,cellPosX-drawMap.viewport_x, cellPosY-drawMap.viewport_y);
    
}

drawMap.UpdateMap=function()
{
    var ctx=document.getElementById("canvas").getContext('2d');

    // clear canvas
    ctx.clearRect(0,0,drawMap.map_w,drawMap.map_h);    
    
    var w,h;
    w=parseInt(drawMap.map_w/drawMap.map_res);  // cell수
    h=parseInt(drawMap.map_h/drawMap.map_res);
    for(y=0;y<h;y++)
    {
        for(x=0;x<w;x++)
        {             
            if(drawMap.mapData[y][x]==0)    // blank
            {
                ctx.fillStyle="rgb(0,0,0)";
            }
            else if(drawMap.mapData[y][x]==1)    // way
            {
                ctx.fillStyle="rgb(255,255,255)";
            }
            else if(drawMap.mapData[y][x]==2)    // node
            {
                ctx.fillStyle="rgb(255,0,0)";
            }
            else if(drawMap.mapData[y][x]==3)    // terminal / table
            {
                ctx.fillStyle="rgb(0,0,255)";
            }
            ctx.fillRect(parseInt(x)*drawMap.map_res+1, parseInt(y)*drawMap.map_res+1,drawMap.map_res-1,drawMap.map_res-1);
            
            ctx.beginPath();
            ctx.rect(parseInt(x)*drawMap.map_res, parseInt(y)*drawMap.map_res,drawMap.map_res,drawMap.map_res);
            ctx.strokeStyle = "rgba(0, 0, 0, 0.5)";
            ctx.stroke();
            ctx.closePath();            
        }
    }

    // 로봇의 현재위치
    /*ctx.fillStyle="rgb(255,0,0)";
    ctx.fillRect(parseInt(cellPosX-drawMap.viewport_x)*drawMap.map_res+1, 
                    parseInt(cellPosY-drawMap.viewport_y)*drawMap.map_res+1,drawMap.map_res-1,drawMap.map_res-1);*/


    //ctx.fillRect(parseInt(posX)*drawMap.map_res+1, parseInt(posY)*drawMap.map_res+1,drawMap.map_res-1,map_res-1);
}

// canvas_w=500, canvas_h=500, w,h=mapsize (10000mm), res(1px=20mm), ptX/ptY(robot좌표(mm)), angle 로봇자세(radian)
drawMap.init=function(canvas_w, canvas_h, w, h, res, ptX, ptY, angle)
{   
    // robot pose
    drawMap.robotPosX=0;   // cell 좌표 x
    drawMap.robotPosY=0;   // cell 좌표 x
    drawMap.robotAngle=180; // 각 top=0 

    drawMap.selTool=0;      // blank

    drawMap.canvas_w=canvas_w;
    drawMap.canvas_h=canvas_h;

    drawMap.map_w=parseInt(w/res);
    drawMap.map_h=parseInt(h/res);
    drawMap.map_res=res;

    console.log(drawMap.map_w + '*' + drawMap.map_h + ',' + drawMap.map_res);
    
    // initialize mapdata
    for(var y=0;y<drawMap.map_h;y++)
    {
        drawMap.mapData[y]=new Array(drawMap.map_w);
    }    
    ClearMap();
    

    drawMap.viewport_w=drawMap.canvas_w/res;
    drawMap.viewport_h=drawMap.canvas_h/res;

    // pos(mm) -> pos(cell's pos)
    drawMap.robotPosX=parseInt(ptX/res);  
    drawMap.robotPosY=parseInt(ptY/res);
    drawMap.robotAngle=angle;

    //viewport를 로봇좌표 ptX, ptY에 설정
    drawMap.viewport_x=(drawMap.robotPosX+parseInt(drawMap.map_w/2))-parseInt((drawMap.viewport_w)/2);
    drawMap.viewport_y=(drawMap.robotPosY+parseInt(drawMap.map_h/2))-parseInt((drawMap.viewport_h)/2);



    // Drawing 함수 주기적으로 호출
    setInterval(function(){
        //Draw(drawMap.robotPosX,drawMap.robotPosY,drawMap.robotAngle);
        
        /*drawMap.mapData[0][0]=0;//-drawMap.mapData[0][0];
        drawMap.mapData[0][1]=1;
        drawMap.mapData[0][2]=2;
        drawMap.mapData[0][3]=3;*/

        /*drawMap.UpdateMap();
        drawMap.DrawRobotPos(drawMap.robotPosX,drawMap.robotPosY,0);   */

        drawMap.DrawCurrentView(drawMap.robotPosX,drawMap.robotPosY,0);

        if(drawMap.robotPosX<30)
            drawMap.robotPosX++;
        else 
            drawMap.robotPosX=0;
        drawMap.robotPosY=0;
    },
    200);
 

    /*

    // 버튼 이벤트 
    document.getElementById("btnIdSelBlank").onclick=function () {
        console.log("blank");
        drawMap.selTool=0;
    }
    document.getElementById("btnIdSelWay").onclick=function () {
        console.log("Way");
        drawMap.selTool=1;
    }
    document.getElementById("btnIdSelNode").onclick=function () {
        console.log("Node");
        drawMap.selTool=2;
    }
    document.getElementById("btnIdSelTerminal").onclick=function () {
        console.log("Terminal");
        drawMap.selTool=3;
    }

    document.getElementById("btnIdNext").onclick=function () {
        console.log("Next");
    }

    document.getElementById("btnIdLeft").onclick=function () {
        console.log("Left");
    }

    document.getElementById("btnIdRight").onclick=function () {
        console.log("Right");
    }

    document.getElementById("btnIdTurn").onclick=function () {
        console.log("Turn");
    }

    document.getElementById("btnIdCancel").onclick=function () {
        console.log("Cancel");
    }

    
    var mouseCapture=0;
    var captureStartX=0;
    var captureStartY=0;

    document.getElementById("canvas").addEventListener("mousedown",function (event) {
        var x=parseInt((event.clientX-canvas.offsetLeft)/map_res);
        var y=parseInt((event.clientY-canvas.offsetTop)/map_res);

        captureStartX=x;
        captureStartY=y;
        
        console.log("canvas mousedown : tools => " + selTool + ',' + x + " , " + y);
        
    })


    document.getElementById("canvas").addEventListener("mouseup",function (event) {
        var x=parseInt((event.clientX-canvas.offsetLeft)/map_res);
        var y=parseInt((event.clientY-canvas.offsetTop)/map_res);

        captureEndX=x;
        captureEndY=y;

        if(captureStartX==captureEndX)
        {
            SetMapVal(x+viewport_x,y+viewport_y,selTool);
        }

        if(captureStartX<captureEndX && captureEndX<viewport_w-1)
        {
            viewport_x+=1;
        }
        else if(captureStartX>captureEndX && viewport_x>0)
        {
            viewport_x-=1;
        }

        if(captureStartY<captureEndY && captureEndY<viewport_h-1)
        {
            viewport_y+=1;
        }
        else if(captureStartY>captureEndY && viewport_y>0)
        {
            viewport_y-=1;
        }

        console.log("canvas mouseup : tools => " + selTool + ',' + x + " , " + y);
        
    })

    */

}


// grid를 그린다. 
drawMap.DrawGrid=function()
{
    var ctx=document.getElementById("canvas").getContext('2d');
              
    gridSize=drawMap.map_res;
    for(var y=0;y<drawMap.map_h;y+=gridSize)
    {
        for(var x=0;x<drawMap.map_w;x+=gridSize)
        { 
        ctx.beginPath();
        ctx.rect(x,y,drawMap.map_res,drawMap.map_res);
        ctx.strokeStyle = "rgba(0, 0, 0, 0.5)";
        ctx.stroke();
        ctx.closePath();
        }
    }  
}