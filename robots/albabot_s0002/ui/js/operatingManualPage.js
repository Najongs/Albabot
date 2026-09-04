var operatingManualPage={};

operatingManualPage.divMain;
operatingManualPage.strRobotPath;
operatingManualPage.inputPathString;
operatingManualPage.divButton;
operatingManualPage.init=function(displayType)
{
    // get element
    operatingManualPage.divMain=document.getElementById("idPageOperatingManual");

    operatingManualPage.inputPathString=document.getElementById("idInputPathString");
    operatingManualPage.strRobotPath='';

    // set style
    operatingManualPage.divMain.setAttribute("style","display:"+displayType+";");

    //operatingManualPage.divButton=document.getElementById("idPageOperatingManual");/*addEventListener("click", function(e)
    //{
    //    console.log("Hello");
    //});*/

    
    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[0].addEventListener("click", function(e){
        operatingManualPage.strRobotPath+='1';
        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath;
        console.log("Next");        
    });

    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[1].addEventListener("click", function(e){
        operatingManualPage.strRobotPath+='424';
        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath;
        console.log("Left");        
    });

    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[2].addEventListener("click", function(e){
        operatingManualPage.strRobotPath+='434';
        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath;
        console.log("Right");        
    });

    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[3].addEventListener("click", function(e){
        operatingManualPage.strRobotPath+='464';
        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath;
        console.log("Turn");        
    });

    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[4].addEventListener("click", function(e){        
        console.log("Start");        

        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath+'0';
        var lenMsg=0;
        if(operatingManualPage.inputPathString.value.length%7==0)
            lenMsg=parseInt(operatingManualPage.inputPathString.value.length/7);
        else
            lenMsg=parseInt(operatingManualPage.inputPathString.value.length/7)+1;
        

        var cmdArray=new Array(lenMsg);
        var idx=0;
        for(var i=0;i<operatingManualPage.inputPathString.value.length;i++)
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
                    cmdArray[idx]=new Array(operatingManualPage.inputPathString.value.length-i+1)
                    cmdArray[idx][0]=idx+1;
                }
            }

            cmdArray[idx][i%7+1]=parseInt(operatingManualPage.inputPathString.value[i]);
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
               
        operatingManualPage.strRobotPath="";
    });


    operatingManualPage.divMain.getElementsByClassName("clsDivBtn")[0].getElementsByClassName("clsBtn")[5].addEventListener("click", function(e){        
        console.log("Cancel");  
        var data2=new Array(2);
        data2[0]=0x00;
        data2[1]=0xff;
        rosConnection.sendTopic(0x100,2,data2);
        delay(50);
               
        operatingManualPage.strRobotPath="";
        operatingManualPage.inputPathString.value=operatingManualPage.strRobotPath;
    });

};