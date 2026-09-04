var configurationPage={};
configurationPage.divMain;


configurationPage.init = function()
{
    document.getElementById("btnConfig_status1").addEventListener("click", function(e){
        console.log("btnConfig_status1");
        configurationPage.SendRobotStatus(0x02);    //rl 0ff  / ig1 on
    });

    document.getElementById("btnConfig_status2").addEventListener("click", function(e){
        console.log("btnConfig_status2");
        configurationPage.SendRobotStatus(0x03);    //rl on / ig1 on
    });

    document.getElementById("btnConfig_status3").addEventListener("click", function(e){
        console.log("btnConfig_status3");
        configurationPage.SendRobotStatus(0x06);    //rl off / ig1 on / left blink on / left blink off
    });

    document.getElementById("btnConfig_status4").addEventListener("click", function(e){
        console.log("btnConfig_status4");
        configurationPage.SendRobotStatus(0x0a);    //rl off / ig1 on / left blink off / left blink on
    });
}

configurationPage.SendRobotStatus=function(status)
{
    var albabotMsg = new ROSLIB.Topic({
        ros : ros,
        name : '/AlbabotMessage',
        messageType : 'albabot_msgs/CanMsg'
    }); 

    var data=new Array(3);
    data[0]=0x00;
    data[1]=0x85;
    data[2]=status;
    
    var commadMsg = new ROSLIB.Message({
        id: 0x100, 
        dlc : 3, 
        data : data//[data[0],data[1],data[2]]
    });
    
    albabotMsg.publish(commadMsg);

    delete data;
}
