
// VARIABLE DECLARACTIONS ****************************************************
var url = "http://blockchain.info/latestblock";

var xhrRequest = function (url, type, callback){
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {   
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


// GET BLOCK HEIGHT FUNCTION ****************************************************
function getBlockHeight() {
  
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object
      var json = JSON.parse(responseText);
      var blockheight = json.height;

      console.log("Blockheight = " + blockheight);
      
      var dictionary = {
        "KEY_BLOCK_HEIGHT": blockheight
      };
  
      // Sending everything to PEBBLE
      console.log("Sending dictionary= " + dictionary);
      Pebble.sendAppMessage(dictionary);
  
    }
  );
}

// EVENT LISTENER JS FUNCTION ****************************************************             
// Listen for when the watchface is opened     
// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");
    getBlockHeight();
  }                     
);
     

// EVENT LISTENER APP MESSAGE FUNCTION ****************************************************
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getBlockHeight();
  }                     
);