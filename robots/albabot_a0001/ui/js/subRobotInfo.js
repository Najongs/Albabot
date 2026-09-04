var subRobotInfo={};

subRobotInfo.listenerRobotInfo;

subRobotInfo.init = function()
{
    subRobotInfo.listenerRobotInfo=new ROSLIB.Topic({
        ros : rosConnection.ros,
        name : '/robot_info',
        messageType : 'albabot_msgs/RobotInfo'
    });


    subRobotInfo.listenerRobotInfo.subscribe(function(message) 
    {
        /*var curTime=new Date().getTime();
        var diffTime = curTime - g_preTime;    
        g_preTime=curTime;
        var strInfo = "";           
        strInfo="Period : " + diffTime + "ms";*/
        strInfo="L enc : " + message.left_enc + "/ R enc : " + message.right_enc;
        //settingPage_Drive.Encoder.innerText="L enc : " + message.left_enc + "/ R enc : " + message.right_enc;

        var n1=message.hall[0].toString(2);       
        var n2= message.hall[1].toString(2);       
        n1="0000000000000000".substr(n1.length)+n1;
        n2="0000000000000000".substr(n2.length)+n2;
        /*settingPage_Drive.Hall2CAN.innerText="Hall2CAN1 : " + n1 + "/ Hall2CAN2 : " + n2;
        settingPage_Drive.sonic.innerText="Sonic : [" + message.sonic[0] + "," + message.sonic[1] + ","+ message.sonic[2] + "," + message.sonic[3] + ","+
                        message.sonic[4] + ","+ message.sonic[5] + ","+ message.sonic[6] + ","+ message.sonic[7] + "]";
        settingPage_Drive.Bat.innerText="Voltage : " + message.batVoltage;        */

        settingPage_Drive.setRobotInfo(message.left_enc, message.right_enc, message.hall, message.sonic[0],message.batVoltage);
        operatingPage.setRobotInfo(message);
        /*document.getElementById("RobotInfo_Period").innerText=strInfo;     
        strInfo = "L enc : "+ message.left_enc;        
        document.getElementById("RobotInfo_EncL").innerText = strInfo;
        strInfo = "R enc : "+ message.right_enc;        
        document.getElementById("RobotInfo_EncR").innerText = strInfo;
        var n=message.hall[0].toString(2);        
        n="0000000000000000".substr(n.length)+n;
        strInfo = "Hall : "+ n;
        document.getElementById("RobotInfo_Hall").innerText = strInfo;*/
        //console.log(strInfo);
    });
}

