var selectPage={};



selectPage.SelectPage=function(value)
{
    switch(value)
    {
        case 0:           
            remotePage.divMain.setAttribute("style","display: block;");
            autonomousPage.divMain.setAttribute("style","display: none;");
            configurationPage.divMain.setAttribute("style","display: none;");
            break;
        case 1:
            remotePage.divMain.setAttribute("style","display: none;");
            autonomousPage.divMain.setAttribute("style","display: block;");
            configurationPage.divMain.setAttribute("style","display: none;");
            break;
        case 2:
            remotePage.divMain.setAttribute("style","display: none;");
            autonomousPage.divMain.setAttribute("style","display: none;");
            configurationPage.divMain.setAttribute("style","display: block;");
            break;
        default:
            break;
    }
    
}