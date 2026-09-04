
window.onload=function()
{
  console.log("onload");

  remotePage.divMain = document.getElementById("content-remote");
  autonomousPage.divMain = document.getElementById("content-autonomous");
  configurationPage.divMain = document.getElementById("content-configuration");

  selectPage.SelectPage(0);

  //initRos('ws://192.168.1.253:9090');
  //initRos('ws://localhost:9090');
  //telebot_ros.initRos('ws://localhost:9090');
  telebot_ros.initRos('ws://impala.redirectme.net:9090');
  //telebot_ros.initRos('ws://albabot.ddns.net:9090');
  //telebot_ros.initRos('ws://192.168.77.4:9090');

  document.getElementById("btnRemote").addEventListener("click", function(e){
    console.log("click:btnRemote");
    selectPage.SelectPage(0);
    document.getElementById("btnRemote")

  });

  document.getElementById("btnAutonomous").addEventListener("click", function(e){
    console.log("click:btnAutonomous");

    selectPage.SelectPage(1); 
  });

  document.getElementById("btnConfiguration").addEventListener("click", function(e){
    console.log("click:btnConfiguration");
    selectPage.SelectPage(2);  
  });

  
  configurationPage.init();

}


var interval;
var interval2;

// joystick
window.addEventListener("gamepadconnected", function(e) {
console.log("Gamepad connected at index %d: %s. %d buttons, %d axes.",
    e.gamepad.index, e.gamepad.id,
    e.gamepad.buttons.length, e.gamepad.axes.length);

    interval = setInterval(gameLoop, 100);
});

window.addEventListener("gamepaddisconnected", function(e) {
    console.log("Gamepad disconnected from index %d: %s",e.gamepad.index, e.gamepad.id);
});

function pollGamepads() 
{
    var gamepads = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : []);
    for (var i = 0; i < gamepads.length; i++) {
        var gp = gamepads[i];
        if (gp) {
        console.log("Gamepad connected at index " + gp.index + ": " + gp.id +
            ". It has " + gp.buttons.length + " buttons and " + gp.axes.length + " axes.");
        //gamepadInfo.innerHTML = "Gamepad connected at index " + gp.index + ": " + gp.id +
        //  ". It has " + gp.buttons.length + " buttons and " + gp.axes.length + " axes.";
        //gameLoop();
        clearInterval(interval);
        }
    }
}



function gameLoop() 
{
    var gamepads = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : []);
    if (!gamepads) {
        return;
    }
    var gp = gamepads[0];
    //console.log(gp.axes[1]+' / '+gp.axes[3]);    
    SendVelocity(gp.axes[1],gp.axes[2]);


    //start = requestAnimationFrame(gameLoop);
}

function SendVelocity(v,w)
{
    var cmd_vel = new ROSLIB.Topic({
        ros : ros,
        name : '/cmd_vel',
        messageType : 'geometry_msgs/Twist'
    }); 
    
    console.log(-v+' / '+-w);  

    if(Math.abs(v)<0.1)
        v=0;
    if(Math.abs(w)<0.1)
        w=0;
    

    var commadMsg = new ROSLIB.Message({
        linear: {
            x: -v,
            y: 0.0,
            z: 0.0},
        angular: {
            x: 0.0,
            y: 0.0,
            z: -w},
    });
    
    cmd_vel.publish(commadMsg);
}