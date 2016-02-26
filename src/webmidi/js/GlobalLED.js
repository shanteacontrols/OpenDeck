/// <reference path="MIDI.js" />

//var _programChangeEnable = window.currentData[1];
//var _MidiID = window.currentData[0];
//var _buttonType = window.currentData[2];
//Get all conf data for buttons



function sendGlobalLEDGet() {
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x04, 0x00, 0xf7]); 
 

 setTimeout(function () {
  editGlobalLedData();
 }, 500);
 var i = 0;
}

function editGlobalLedData() {
 if (window.currentData != undefined) {
  //features
  var _totalLED = getDataForIndex(0, window.currentData[0]);
  var _blinkTime = getDataForIndex(1, window.currentData[0]);
  var _startUpSwitchTime = getDataForIndex(2, window.currentData[0]);
  var _startUpRoutine = getDataForIndex(3, window.currentData[0]);
  var _fadeTime = getDataForIndex(4, window.currentData[0]);
  
  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText('Global setup'), "LED"));
  _edit.appendChild(addBlock(generateRange('total-led', _totalLED, 0, 48), "Total LED number"));
  _edit.appendChild(addBlock(generateRange('blink-time', _blinkTime, 0, 15, 100), "Blink time")); // *100
  _edit.appendChild(addBlock(generateRange('startup-switch', _startUpSwitchTime, 0, 120, 10), "Start-up switch time")); //10
  _edit.appendChild(addBlock(generateRange('startup-routine', _startUpRoutine, 0, 5), "Start-up routine")); 
  _edit.appendChild(addBlock(generateRange('fade', _fadeTime, 0, 10), "Fade time"));
    

  _edit.appendChild(addBlock(setSaveGlobalLED()), '');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });


 }
}


function setSaveGlobalLED() {
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveGlobalLed();"><span>Save</span></a>';
}




function sendMessageSaveGlobalLed() {


 var _totalLED = getValueRange('total-led');
 var _blinkTime = getValueRange('blink-time');
 var _startUpSwitchTime = getValueRange('startup-switch');
 var _startUpRoutine = getValueRange('startup-routine');
 var _fadeTime = getValueRange('fade');

 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x01, 0x04, 0x00, _totalLED, _blinkTime, _startUpSwitchTime, _startUpRoutine, _fadeTime, 0xf7]);

 


}
