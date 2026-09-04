var loadingPage={};

loadingPage.divMain;
loadingPage.url;
loadingPage.isLoad;

loadingPage.init=function(displayType)
{
    loadingPage.isLoad=false;
    // get element
    loadingPage.divMain=document.getElementById("idPageLoading");

    // set style
    loadingPage.divMain.setAttribute("style","display:"+displayType+";");

    //loadingPage.connectDlg();

    console.log("start");
    //while(loadingPage.isLoad==false);
    //console.log(loadingPage.url);

    //loadingPage.url='ws://localhost:8080';
    //loadingPage.url='ws://192.168.1.80:8080';
    
    //loadingPage.isLoad=true;

    document.getElementById("btnIdConnection").addEventListener("click", function(e){

        var inputUrl=document.getElementById("inputIdUrl");        

        console.log(inputUrl.value);
        
        //loadingPage.url='ws://192.168.1.80:8080';
        loadingPage.url=inputUrl.value;
        loadingPage.isLoad=true;

        rosConnection.init(loadingPage.url);

        // ros topic 
        subscribeTest.init();
        subRobotInfo.init();

        selectPage.toggleDiv(2);
    })


    //loadingPage.makeOenFile();
    //readTextFile("files/mapFile.txt");
    
};

loadingPage.connectDlg=function()
{
    var divWrapper = document.createElement('div');
    var btnConnect = document.createElement('button');
    var inputUrl=document.createElement('input');

    btnConnect.textContent="연결하기";
    btnConnect.setAttribute("style","width:100px; height:30px;");    
    divWrapper.append(inputUrl);
    divWrapper.append(btnConnect);

    loadingPage.divMain.append(divWrapper);

    btnConnect.addEventListener('click',function(e){
        if(inputUrl.value!="")
        {
            //loadingPage.isLoad=true;
            console.log(loadingPage.url);
        }
    })
}


loadingPage.mapData;

function readTextFile(file)
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function ()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status == 0)
            {
                var allText = rawFile.responseText;
                var array = allText.split('\n');

                var arrayHead = array[0].split(" ");
                var sizeX, sizeY;
                sizeX=parseInt(arrayHead[0]);
                sizeY=parseInt(arrayHead[1]);
                
                delete loadingPage.mapData;
                loadingPage.mapData=new Array();

                //console.log("size : " + parseInt(arrayHead) + ',' + parseInt(arrayHead) );
                for(var i=0;i<sizeY;i++)
                {
                    loadingPage.mapData[i]=new Array(2);              
                    
                    var arrayHead = array[i+1].split(" ");
                    for(var j=0;j<sizeX;j++)
                    {
                        loadingPage.mapData[i][j]=parseInt(arrayHead[j]);
                    }
                }

                console.log("num: " + sizeX +','+sizeY);
                console.log(loadingPage.mapData);
                

            }
        }
    }
    rawFile.send(null);
}




loadingPage.makeView = function()
{
    btnFileOpen=document.createElement('button');
    
    btnFileOpen.textContent="Open File";
    loadingPage.divMain.append(btnFileOpen);

    btnFileOpen.addEventListener("click",function(e){


    });
}

loadingPage.makeOenFile=function()
{
    inputOpenFile=document.createElement('input');

    inputOpenFile.setAttribute("type","file");
    inputOpenFile.setAttribute("accept", "text/plain, text/html, .js");
    inputOpenFile.setAttribute("webkitdirectory","false");

    loadingPage.divMain.append(inputOpenFile);

    console.log(inputOpenFile.files[0]);


/*
    //inputOpenFile.click();
    inputOpenFile.addEventListener("change", function(e){
        console.log(e.target.files[0]);

    });
    /*inputOpenFile.addEventListener("click",function(e){


    });*/
}
