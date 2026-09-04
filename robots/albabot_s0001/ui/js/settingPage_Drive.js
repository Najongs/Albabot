var settingPage_Drive={};

// <div>
settingPage_Drive.divMain;
// <p>
settingPage_Drive.Encoder;
settingPage_Drive.Hall2CAN;
settingPage_Drive.Bat;
settingPage_Drive.sonic;

// <button>
settingPage_Drive.btnNext;
settingPage_Drive.btnLeft; 
settingPage_Drive.btnRight;
settingPage_Drive.btnTurn; 
settingPage_Drive.btnCancel;
settingPage_Drive.btnModeAgv;   
settingPage_Drive.btnModeRemote;

// <input>
settingPage_Drive.inputPathplan;

//
settingPage_Drive.divRobotInfo;
settingPage_Drive.pRobotInfoArr;
settingPage_Drive.pRobotInfoArrName;

settingPage_Drive.divCommandButton;
settingPage_Drive.btnCommandArr;
settingPage_Drive.btnCommandArrName;

settingPage_Drive.init = function()
{
    settingPage_Drive.divMain=document.getElementById("divIdDriveRobotInfo");

    //settingPage_Drive.makeView();

    //settingPage_Drive.makeCommandKey();
    settingPage_Drive.makeCommandButton();
    settingPage_Drive.makeRobotInfoForm();
    //set handle function
    settingPage_Drive.setHandler();
}

settingPage_Drive.makeCommandButton=function()
{   
    var styleBtnCommand="width:100px;height:30px;";
    var styleDivCommandButton="position: absolute; top:50%;left:50%;border:solid 1px gray; width:400px; height:300px;\
                    margin-left: -200px; margin-top: -150px;";
    var btnStyle1="width:100px; background-color: #123456; border: none; color:#fff; padding: 15px 0; \
                    text-align: center; text-decoration: none; border-radius:10px;\
                    display: inline-block; font-size: 15px;  margin: 4px; cursor: pointer;";

    var btnStyle2="width:100px; background-color: #654321; border: none; color:#fff; padding: 15px 0; \
                    text-align: center; text-decoration: none; border-radius:10px;\
                    display: inline-block; font-size: 15px;  margin: 4px; cursor: pointer;";      

    var divTemp=new Array(4);
    
    settingPage_Drive.btnCommandArrName=["Next","Left","Cancel","Right","Turn", "Mode:AGV","Mode:Remote"];
    settingPage_Drive.btnCommandArr=new Array(settingPage_Drive.btnCommandArrName.length);

    // create 
    settingPage_Drive.divCommandButton=document.createElement('div');
    settingPage_Drive.divCommandButton.setAttribute("style", styleDivCommandButton);
    // create
    for(var i=0;i<divTemp.length;i++)
        divTemp[i]=document.createElement('div');   
           
    for(var i=0;i<settingPage_Drive.btnCommandArr.length;i++)
    {
        settingPage_Drive.btnCommandArr[i]=document.createElement('button');        
        settingPage_Drive.btnCommandArr[i].setAttribute("style",btnStyle1);
        settingPage_Drive.btnCommandArr[i].textContent=settingPage_Drive.btnCommandArrName[i];
        settingPage_Drive.divCommandButton.append(settingPage_Drive.btnCommandArr[i]);
    }

    // append
    divTemp[0].append(settingPage_Drive.btnCommandArr[0]);
    divTemp[1].append(settingPage_Drive.btnCommandArr[1]);
    divTemp[1].append(settingPage_Drive.btnCommandArr[2]);
    divTemp[1].append(settingPage_Drive.btnCommandArr[3]);
    divTemp[2].append(settingPage_Drive.btnCommandArr[4]);
    divTemp[3].append(settingPage_Drive.btnCommandArr[5]);
    divTemp[3].append(settingPage_Drive.btnCommandArr[6]);


    settingPage_Drive.divCommandButton.append(divTemp[0]);
    settingPage_Drive.divCommandButton.append(divTemp[1]);
    settingPage_Drive.divCommandButton.append(divTemp[2]);
    settingPage_Drive.divCommandButton.append(divTemp[3]);
    
    settingPage_Drive.divMain.append(settingPage_Drive.divCommandButton);
}

settingPage_Drive.makeRobotInfoForm=function()
{    
    settingPage_Drive.pRobotInfoArrName=["Enc L :","Enc R :","Hall :","Sonic :","Bat :"];
    settingPage_Drive.pRobotInfoArr=new Array(settingPage_Drive.pRobotInfoArrName.length);

    settingPage_Drive.divRobotInfo=document.createElement('div');
    for(var i=0;i<settingPage_Drive.pRobotInfoArr.length;i++)
    {
        settingPage_Drive.pRobotInfoArr[i]=document.createElement('p');        
        settingPage_Drive.divRobotInfo.append(settingPage_Drive.pRobotInfoArr[i]);
    }
    //settingPage_Drive.divRobotInfo.append(settingPage_Drive.pRobotInfoArr);
    settingPage_Drive.divMain.append(settingPage_Drive.divRobotInfo);
}

settingPage_Drive.setRobotInfo=function(encL,encR,hall,sonic,bat)
{    
    var data=new Array(5);
    data[0]=encL;
    data[1]=encR;
    data[2]=hall;
    data[3]=sonic;
    data[4]=bat;

    for(var i=0;i<settingPage_Drive.pRobotInfoArr.length;i++)
    {
        settingPage_Drive.pRobotInfoArr[i].innerText=settingPage_Drive.pRobotInfoArrName[i] + data[i];
    }
}

function delay(time) {
    var d1 = new Date();
    var d2 = new Date();
    while (d2.valueOf() < d1.valueOf() + time) 
    {
        d2 = new Date();
    }
}
    

settingPage_Drive.setHandler=function()
{
    settingPage_Drive.btnCommandArr[0].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0x00;
        rosConnection.sendTopic(0x100,2,data);
    });

    settingPage_Drive.btnCommandArr[1].addEventListener("click", function(e){
        var data=new Array(8);
        //data[0]=0x00;
        //data[1]=0x02;
        //rosConnection.sendTopic(0x100,2,data);

        //var data=new Array(6);
        data[0]=0x01;
        data[1]=0x04;
        data[2]=0x02;
        data[3]=0x04;
        data[4]=0x03;
        data[5]=0x04;
        data[6]=0x02;
        data[7]=0x04;
        rosConnection.sendTopic(0x100,8,data);
        delete data;
        delay(50);

        data=new Array(5);        
        data[0]=0x02;
        data[1]=0x02;
        data[2]=0x04;
        data[3]=0x02;
        data[4]=0x00;
        rosConnection.sendTopic(0x100,5,data);
        delete data;
        delay(50);

        data=new Array(2);
        data[0]=0x00;
        data[1]=0xff;
        rosConnection.sendTopic(0x100,2,data);
        delete data;
        delay(50);

        data=new Array(8);
        data[0]=0x01;
        data[1]=0x04;
        data[2]=0x02;
        data[3]=0x04;
        data[4]=0x03;
        data[5]=0x04;
        data[6]=0x02;
        data[7]=0x04;
        rosConnection.sendTopic(0x100,8,data);
        delete data;
        delay(50);

        data=new Array(5);        
        data[0]=0x02;
        data[1]=0x02;
        data[2]=0x04;
        data[3]=0x02;
        data[4]=0x00;
        rosConnection.sendTopic(0x100,5,data);
        delete data;
        delay(50);

        var data2=new Array(2);
        data2[0]=0x00;
        data2[1]=0x00;
        rosConnection.sendTopic(0x100,2,data2);
        delay(50);

        
        /*data[0]=0x00;
        data[1]=0x00;
        rosConnection.sendTopic(0x100,2,data);*/



    });

    settingPage_Drive.btnCommandArr[2].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0xff;
        rosConnection.sendTopic(0x100,2,data);

    });

    settingPage_Drive.btnCommandArr[3].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0x03;
        rosConnection.sendTopic(0x100,2,data);
    });

    settingPage_Drive.btnCommandArr[4].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0x4;
        rosConnection.sendTopic(0x100,2,data);
    });

    settingPage_Drive.btnCommandArr[5].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0x31;
        data[2]=0x01;   // agv 1 , remote 0
        rosConnection.sendTopic(0x100,3,data);
    });

    settingPage_Drive.btnCommandArr[6].addEventListener("click", function(e){
        var data=new Array(2);
        data[0]=0x00;
        data[1]=0x31;
        data[2]=0x00;   // agv 1 , remote 0
        rosConnection.sendTopic(0x100,3,data);
    });
}

