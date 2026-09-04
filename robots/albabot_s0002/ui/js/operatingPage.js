var operatingPage={};

operatingPage.divMain;
operatingPage.opmode;
operatingPage.preTime;

operatingPage.init=function(displayType)
{
    // get element
    operatingPage.divMain=document.getElementById("idPageOperating");
    
    // set style
    operatingPage.divMain.setAttribute("style","display:"+displayType+";");

    selectTablePage.init(["T1","T2","T3","T4","T5"]);  
    
    operatingPage.divMain.append(selectTablePage.dlgMain);
    drawMap.init(500,500,10000,10000,50,0,0,0);
    //drawMap.DrawGrid();

    operatingPage.opmode=4;  
};

operatingPage.setRobotInfo=function(msg)
{
    var curTime=new Date().getTime();
    var diffTime = curTime - operatingPage.preTime;    
    operatingPage.preTime=curTime;
    
    //console.log("Period : " + diffTime + "ms");

    if(operatingPage.opmode==0)
    {
        operatingPage.opmode=1;
        selectTablePage.dlgMain.showModal();        
        //selectTablePage.showDlg(0);
        document.getElementById("pIdTest").innerText="현재위치를 선택하세요!";
    }
    else if(operatingPage.opmode==1)
    {
        console.log(selectTablePage.dlgMain.returnValue);
        if(selectTablePage.dlgMain.returnValue=="T1")
        {
            operatingPage.opmode=2;
        }
    }
    else if(operatingPage.opmode==2)
    {
        operatingPage.opmode=3;
        selectTablePage.dlgMain.showModal();   
        document.getElementById("pIdTest").innerText="방향를 선택하세요!";     
        //selectTablePage.showDlg(0);
    }
    else if(operatingPage.opmode==3)
    {
        console.log(selectTablePage.dlgMain.returnValue);
        if(selectTablePage.dlgMain.returnValue=="T1")
        {
            operatingPage.opmode=4;
        }
    }
    else if(operatingPage.opmode==4)
    {
        var strInfo="L enc : " + msg.left_enc + "/ R enc : " + msg.right_enc + "/ Bat : "+msg.batVoltage;
        document.getElementById("pIdTest").innerText=strInfo;
        //console.log(strInfo);
    }
       
}


