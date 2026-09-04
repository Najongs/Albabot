
var selectPage={};

selectPage.divMain;

selectPage.btnSelectorLoad;
selectPage.btnSelectorOperate;
selectPage.btnSelectorSet;

selectPage.init=function()
{
    selectPage.btnSelectorLoad=document.getElementById("btnSelectorLoad");
    selectPage.btnSelectorOperate=document.getElementById("btnSelectorOperate");
    selectPage.btnSelectorSet=document.getElementById("btnSelectorSet");

    
    
    selectPage.btnSelectorLoad.addEventListener("click", function(e){
        console.log("Load");
        selectPage.toggleDiv(0);
    });

    selectPage.btnSelectorOperate.addEventListener("click", function(e){
        console.log("Operate");
        selectPage.toggleDiv(1);
    });

    selectPage.btnSelectorSet.addEventListener("click", function(e){
        console.log("Set");
        selectPage.toggleDiv(2);
    });
}

selectPage.toggleDiv=function(value)
{
    switch(value)
    {
        case 0:
           
            loadingPage.divMain.setAttribute("style","display: block;");
            operatingPage.divMain.setAttribute("style","display: none;");
            settingPage.divMain.setAttribute("style","display: none;");
            break;
        case 1:
            loadingPage.divMain.setAttribute("style","display: none;");
            operatingPage.divMain.setAttribute("style","display: block;");
            settingPage.divMain.setAttribute("style","display: none;");
            break;
        case 2:
            loadingPage.divMain.setAttribute("style","display: none;");
            operatingPage.divMain.setAttribute("style","display: none;");
            settingPage.divMain.setAttribute("style","display: block;");
            break;
        default:
            break;
    }
    
}