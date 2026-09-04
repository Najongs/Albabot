
var subscribeTest = {};

subscribeTest.topicTest;


subscribeTest.init = function()
{
    var tempP = document.getElementById("pIdTest");
    //var tempP = document.getElementsByClassName("page_operating")
    subscribeTest.topicTest=new ROSLIB.Topic({
        ros : rosConnection.ros,
        name : '/test',
        messageType : 'std_msgs/Byte'
    });

    subscribeTest.topicTest.subscribe(function(msg){    
        var upData;
        if(msg.data<0)
            upData=msg.data+256;
        else
            upData=msg.data;
            
        //console.log("Topic[byte] => 0x" + upData.toString(16));
        //document.getElementById("pIdTest").innerText=msg.data;        
        tempP.innerText="[data]0x" + upData.toString(16);//parseInt(msg.data,16);
    });
}

