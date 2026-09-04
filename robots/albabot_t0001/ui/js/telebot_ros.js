var telebot_ros={};
var ros;
var ros_url;

var encValue1;
var encValue2;

telebot_ros.initRos=function(url)
{
    ros=new ROSLIB.Ros({
        url:url
    });

    ros.on('connection', function() 
    { 
        console.log('Connected to websocket server.');

        // D435i 영상 토픽
        var imageTopic = new ROSLIB.Topic({
        ros : ros,
        name : '/camera/color/image_raw/compressed',
        messageType : 'sensor_msgs/CompressedImage'
        });
        
        // 구독 후 이미지 디스플레이
        imageTopic.subscribe(function(message) 
        {
            document.getElementById('left_stream1').src = "data:image/jpg;base64," + message.data;
            
        });

        var imageTopic2 = new ROSLIB.Topic({
            ros : ros,
            name : '/thermal/image/compressed',
            messageType : 'sensor_msgs/CompressedImage'
            });
            
            // 구독 후 이미지 디스플레이
            imageTopic2.subscribe(function(message) 
            {
              console.log(message);
                document.getElementById('thermal_stream').src = "data:image/png;base64," + message.data;
                
            });

        // 로봇 주행 토픽
        var cmd_vel = new ROSLIB.Topic({
            ros : ros,
            name : '/cmd_vel',
            messageType : 'geometry_msgs/Twist'
          }); 

        var topicRobotInfo=new ROSLIB.Topic({
            ros : ros,
            name : '/robot_info',
            messageType : 'albabot_msgs/RobotInfo'
          });  
          
          var preEncLeft;
          var preEncRight;

          topicRobotInfo.subscribe(function(message){
            var vel1, vel2;
            vel1=message.left_enc-preEncLeft;
            vel2=message.right_enc-preEncRight;

            // 1. 주행중인지 확인
            /*if( Math.abs(vel1+vel2/2.0)<5.0)
                console.log("stop | " + "v_l : " + vel1 + "v_r : " + vel2);
            else
                console.log("on driving | " + "v_l : " + vel1 + "v_r : " + vel2);*/
            preEncLeft=message.left_enc;
            preEncRight=message.right_enc;

            // 2. 배터리 확인
            var voltage=(message.batVoltage-3020)/((3840-3020)/6.0) + 22.0;
            //console.log("배터리전압 : " + voltage);

            if(voltage>28.0)
                document.getElementById('img-batStatus').src="img/icon/battery7.png";
            else if(voltage>27.0)
                document.getElementById('img-batStatus').src="img/icon/battery6.png";
            else if(voltage>26.0)
                document.getElementById('img-batStatus').src="img/icon/battery5.png";
            else if(voltage>25.0)
                document.getElementById('img-batStatus').src="img/icon/battery4.png";
            else if(voltage>24.0)
                document.getElementById('img-batStatus').src="img/icon/battery3.png";
            else if(voltage>23.0)
                document.getElementById('img-batStatus').src="img/icon/battery2.png";
            else
                document.getElementById('img-batStatus').src="img/icon/battery1.png";
        
            document.getElementById('img-wifi').src="img/icon/wifi1.png";

            
                             
          })
    
        // Create the main viewer.
        var viewer3d = new ROS3D.Viewer({
            divID : 'remote-map',
            width : 500,
            height : 400,
            antialias : true
        });

        var tfClient = new ROSLIB.TFClient({
            ros : ros,
            angularThres : 0.01,
            transThres : 0.01,
            rate : 10.0,
            //serverName: 'ws://impala.redirectme.net:9090',
            fixedFrame : 'map'
        });

        // Setup the map client.
        var gridClient1 = new ROS3D.OccupancyGridClient({
            ros : ros,
            continuous : 'true',
            topic: 'map',
            opacity: 0.7,
            rootObject : viewer3d.scene
        });

                
        var grid=new ROS3D.Grid({
            num_cells : 10,
            cellSize : 0.5,
            color: '#dddddd'
        });
        viewer3d.addObject(grid);


        

    });

    ros.on('error', function(error) { 
    console.log('Error connecting to websocket server: ', error); 
    window.alert('Error connecting to websocket server'); 
    });
    
    ros.on('close', function() { 
    console.log('Connection to websocket server closed.');
    });
}
