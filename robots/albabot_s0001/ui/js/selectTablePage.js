var selectTablePage={};

selectTablePage.dlgMain;

selectTablePage.init=function(tableNameArr)
{
    selectTablePage.dlgMain=document.createElement('dialog');    
    var btnArr = new Array(tableNameArr.length);

    selectTablePage.dlgMain.append(document.createElement('p'));
    for(var i=0;i<tableNameArr.length;i++)
    {
        btnArr[i]=document.createElement('button');
        btnArr[i].textContent=tableNameArr[i];
        if(i%3==2)
        {                   
            selectTablePage.dlgMain.append(btnArr[i]);
            selectTablePage.dlgMain.append(document.createElement('p'));
        }
        else
        {
            selectTablePage.dlgMain.append(btnArr[i]);
        }
    }

    //selectTablePage.selectTablePage.dlgMainMain=selectTablePage.dlgMain;
    console.log("Hello:sel table");

    selectTablePage.dlgMain.addEventListener("click",function(e){ 
                                        
        if(e.target.textContent==tableNameArr[0])
        {
            selectTablePage.dlgMain.returnValue=e.target.textContent;
            console.log(e.target.textContent + " button clicked - " + (new Date()).toString());
            selectTablePage.dlgMain.close();
        }
    });
}

selectTablePage.showDlg=function(opmode)
{
    if(opmode==0)
    {
        //alert("현재위치를 선택하세요!");
        selectTablePage.dlgMain.showDlg();
    }
    else if(opmode==1)
    {
        //alert("방향을 선택하세요!");
        selectTablePage.dlgMain.showDlg();
    }

}