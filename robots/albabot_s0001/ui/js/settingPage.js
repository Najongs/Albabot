var settingPage={};

settingPage.divMain;
settingPage.topicAlbabotMessage;

settingPage.btnIdMap;
settingPage.btnIdDrive;
settingPage.btnIdParam;
settingPage.btnIdUrl;

settingPage.divIdMap;
settingPage.divIdDrive;
settingPage.divIdParam;
settingPage.divIdUrl;


settingPage.init=function(displayType)
{
    // get element
    settingPage.divMain=document.getElementById("idPageSetting");
    settingPage.btnIdMap=document.getElementById("btnIdMap");
    settingPage.btnIdDrive=document.getElementById("btnIdDrive");
    settingPage.btnIdParam=document.getElementById("btnIdParam");
    settingPage.btnIdUrl=document.getElementById("btnIdUrl");

    settingPage.divIdMap=document.getElementById("idPageSettingViewMap");
    settingPage.divIdDrive=document.getElementById("idPageSettingViewDrive");
    settingPage.divIdParam=document.getElementById("idPageSettingViewParam");
    settingPage.divIdUrl=document.getElementById("idPageSettingViewUrl");


    settingPage.btnIdMap.addEventListener("click", function(e){
        settingPage.clickedMapButton(e);
    });
    
    settingPage.btnIdDrive.addEventListener("click", function(e){
        settingPage.clickedDriveButton(e);
    });
    
    settingPage.btnIdParam.addEventListener("click", function(e){
        settingPage.clickedParamButton(e);

    });
    
    settingPage.btnIdUrl.addEventListener("click", function(e){
        settingPage.clickedUrlButton(e);
    });

    // set style
    settingPage.divMain.setAttribute("style","display:"+displayType+";");
    settingPage.toggleDiv(0);

    settingPage_Drive.init();
};


settingPage.toggleDiv=function(value)
{
    
    // toggle Div
    settingPage.divIdMap.setAttribute("style", "display:none;");
    settingPage.divIdDrive.setAttribute("style", "display:none;");
    settingPage.divIdParam.setAttribute("style", "display:none;");
    settingPage.divIdUrl.setAttribute("style", "display:none;");

    switch(value)
    {
        case 0:
            settingPage.divIdMap.setAttribute("style", "display:block;");
            break;
        case 1:
            settingPage.divIdDrive.setAttribute("style", "display:block;");
            break;
        case 2:
            settingPage.divIdParam.setAttribute("style", "display:block;");
            break;
        case 3:
            settingPage.divIdUrl.setAttribute("style", "display:block;");
            break;
        default:
            break;

    }
}


settingPage.clickedMapButton = function (e)
{   
    
    // toggle Div
    settingPage.toggleDiv(0);
    
    console.log("btnIdMap");
    


}

settingPage.clickedDriveButton = function (e)
{    
    // toggle Div
    settingPage.toggleDiv(1);


    console.log("btnIdDrive");
    
}

settingPage.clickedParamButton = function (e)
{    
    // toggle Div
    settingPage.toggleDiv(2);
}

settingPage.clickedUrlButton = function (e)
{    
    // toggle Div
    settingPage.toggleDiv(3);
    console.log("btnIdUrl");

    settingPage_Url.init();
    
    
}
