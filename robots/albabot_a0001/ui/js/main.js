window.onload=function()
{   
    //rosConnection.init('ws://192.168.1.103:8080');

    // init page
    selectPage.init(); 

    loadingPage.init("block");
    operatingPage.init("none");
    settingPage.init("none");

    
    /*
    rosConnection.init(loadingPage.url);
    //rosConnection.init('ws://localhost:8080');


    // ros topic 
    subscribeTest.init();
    subRobotInfo.init();

    // 
    
    operatingPage.init("none");
    settingPage.init("none");

    */

       
}