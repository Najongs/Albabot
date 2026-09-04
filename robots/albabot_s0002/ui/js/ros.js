var rosConnection={};

rosConnection.ros;
rosConnection.topicAlbabotMessage;

rosConnection.init=function(value)
{
    rosConnection.ros=new ROSLIB.Ros({
        //url : 'ws://localhost:8080'
        url : value
        //url : 'ws://192.168.1.80:8080'
        //url : 'ws://albabot.ddns.net:8080'
        //url : 'ws://impala.redirectme.net:8080'
    });



    rosConnection.ros.on('connection', function() {
        console.log('Connected to websocket server.');
        
        
        rosConnection.topicAlbabotMessage=new ROSLIB.Topic({
            ros : rosConnection.ros,
            name : '/AlbabotMessage',
            messageType : 'albabot_msgs/CanMsg'
        });

        // ros 연결 후 명령어를 하나 보내야 경고가 뜨지 않음. (이유모름)
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0xff;
        rosConnection.sendTopic(0x100,2,data);
        delete data;

    });

    rosConnection.ros.on('error', function(error) {
        console.log('Error connecting to websocket server: ', error);
    });

    rosConnection.ros.on('close', function() {
        console.log('Connection to websocket server closed.');

        delete rosConnection.topicAlbabotMessage;
        delete subRobotInfo.listenerRobotInfo;
        delete subscribeTest.topicTest;

    });


    
}


rosConnection.sendTopic=function(id,len,data)
{
    var commadMsg = new ROSLIB.Message({
        id: id, 
        dlc : len, 
        data : data//[data[0],data[1],data[2]]
    });
    
    rosConnection.topicAlbabotMessage.publish(commadMsg);
}


