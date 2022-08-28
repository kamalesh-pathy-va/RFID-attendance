//This is the google script file linked with the google sheet and this act as the server side code.

function doGet(e){
  Logger.log("--- doGet ---");
 
 var tag = "",
     value = "";
 
  try {
 
    // this helps during debuggin
    if (e == null){
      e={};
      e.parameters = {tag:"test",value:"-1"};
    }
 
    tag = e.parameters.tag;
    value = e.parameters.value;
 
    // save the data to spreadsheet
    save_data(tag, value);
 
 
    return ContentService.createTextOutput("Wrote:\n  tag: " + tag + "\n  value: " + value);
 
  } catch(error) { 
    Logger.log(error);    
    return ContentService.createTextOutput("oops...." + error.message 
                                            + "\n" + new Date() 
                                            + "\ntag: " + tag +
                                            + "\nvalue: " + value);
  }  
}
 
// Method to save given data to a sheet
function save_data(tag, value){
  Logger.log("--- save_data ---"); 
 
 
  try {
    var dateTime = new Date();
 
    var ss = SpreadsheetApp.openById("18dKccvkujstnayy-vkotMiRUBJ82-6N3LD6TeOR3_Gs");
    Logger.log(ss);
    var dataLoggerSheet = ss.getSheetByName("Sheet1");
    Logger.log(dataLoggerSheet);
 
    var row = dataLoggerSheet.getLastRow() + 1;
 
    dataLoggerSheet.getRange("A" + row).setValue(row -1); // ID
    dataLoggerSheet.getRange("B" + row).setValue(dateTime); // dateTime
    dataLoggerSheet.getRange("C" + row).setValue(tag); // tag
    dataLoggerSheet.getRange("D" + row).setValue(value); // value
 
  }
 
  catch(error) {
    Logger.log(error);
  }
 
  Logger.log("--- save_data end---"); 
}