
var settingPage_Url ={}

settingPage_Url.btnAddNext;
settingPage_Url.btnAddLeft;
settingPage_Url.btnAddRight;
settingPage_Url.btnAddTurn;
settingPage_Url.btnSendPath;

settingPage_Url.inputPathString;
settingPage_Url.strRobotPath;
settingPage_Url.init = function()
{    
    settingPage_Url.strRobotPath="";

    settingPage_Url.btnAddNext=document.getElementById('btnIdAddNext');
    settingPage_Url.btnAddLeft=document.getElementById('btnIdAddLeft');
    settingPage_Url.btnAddRight=document.getElementById('btnIdAddRight');
    settingPage_Url.btnAddTurn=document.getElementById('btnIdAddTurn');
    settingPage_Url.btnSendPath=document.getElementById('btnIdSendPath');

    settingPage_Url.inputPathString=document.getElementById("inputPathString");

    //set handle function
    settingPage_Url.setHandler();
}

settingPage_Url.setHandler=function()
{
    settingPage_Url.btnAddNext.addEventListener('click', function(e){
        settingPage_Url.strRobotPath+='1';
        settingPage_Url.inputPathString.value=settingPage_Url.strRobotPath;

        console.log(settingPage_Url.strRobotPath);
    });

    settingPage_Url.btnAddLeft.addEventListener('click', function(e){
        settingPage_Url.strRobotPath+='424';
        settingPage_Url.inputPathString.value=settingPage_Url.strRobotPath;
        console.log(settingPage_Url.strRobotPath);
    });

    settingPage_Url.btnAddRight.addEventListener('click', function(e){
        settingPage_Url.strRobotPath+='434';
        settingPage_Url.inputPathString.value=settingPage_Url.strRobotPath;
        console.log(settingPage_Url.strRobotPath);
    });

    settingPage_Url.btnAddTurn.addEventListener('click', function(e){
        settingPage_Url.strRobotPath+='464';
        settingPage_Url.inputPathString.value=settingPage_Url.strRobotPath;
        console.log(settingPage_Url.strRobotPath);
    });

    settingPage_Url.btnSendPath.addEventListener("click",function(e){
        
        settingPage_Url.inputPathString.value=settingPage_Url.strRobotPath+'0';
        var lenMsg=0;
        if(settingPage_Url.inputPathString.value.length%7==0)
            lenMsg=parseInt(settingPage_Url.inputPathString.value.length/7);
        else
            lenMsg=parseInt(settingPage_Url.inputPathString.value.length/7)+1;
        

        var cmdArray=new Array(lenMsg);
        var idx=0;
        for(var i=0;i<settingPage_Url.inputPathString.value.length;i++)
        {
            idx=parseInt(i/7);
            if(i%7==0)
            {
                if(idx<lenMsg-1)
                {
                    cmdArray[idx]=new Array(7+1);
                    cmdArray[idx][0]=idx+1;
                }
                else
                {
                    cmdArray[idx]=new Array(settingPage_Url.inputPathString.value.length-i+1)
                    cmdArray[idx][0]=idx+1;
                }
            }

            cmdArray[idx][i%7+1]=parseInt(settingPage_Url.inputPathString.value[i]);
        }


        for(var i=0;i<lenMsg;i++)
        {
            console.log(cmdArray[i] + ' / len : ' + cmdArray[i].length);  
            rosConnection.sendTopic(0x100,cmdArray[i].length,cmdArray[i]);
            delete cmdArray[i];        
            delay(50);            
        }

        var data2=new Array(2);
        data2[0]=0x00;
        data2[1]=0x00;
        rosConnection.sendTopic(0x100,2,data2);
        delay(50);
                       
        settingPage_Url.strRobotPath="";


    });


    




}
