var _ledCount = 48;

function sendLedGet() {

 //window.currentAction = ENUM_Action.GET_Conf_Buttons;
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x04, 0x01, 0xf7]); //Activation notes
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x04, 0x02, 0xf7]); //Start-up numbers
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x04, 0x03, 0xf7]); //RGB led enabled state
 setTimeout(function () {
  fillAllLed();
 }, 500);

}


function fillAllLed() {

 resetContent();
 for (var i = 0; i < _ledCount; i++) {
  var _a = document.createElement('a');
  _a.setAttribute('href', "javascript:void(0);");
  _a.setAttribute('class', 'buttonOuter');
  _a.setAttribute('onclick', 'editLedData(' + i + ');')
  _a.innerHTML += '<span class="buttonInner">' + (i + 1) + '</span>';

  var temp = getDataForIndex(i, window.currentData[0]);
  _a.innerHTML += '<span class="buttonInnerLevel0">Activation note: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[1]);
  
  _a.innerHTML += '<span class="buttonInnerLevel1">Start-up number: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[2]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';
  _a.innerHTML += '<span class="buttonInnerLevel2">RGB: ' + temp + '</span>';
  temp = null;

  
  document.getElementById('content').appendChild(_a);
 }
}



function editLedData(_index) {
 if (window.currentData != undefined) {

  var _activationNote = getDataForIndex(_index, window.currentData[0]);
  var _startupNumber = getDataForIndex(_index, window.currentData[1]);
  var _rgbEnable = getDataForIndex(_index, window.currentData[2]);
  

  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText(_index + 1), "LED"));
  _edit.appendChild(addBlock(generateRange('activation-note', _activationNote, 0, 127), "Activation note"));
  _edit.appendChild(addBlock(generateRange('startup-number', _startupNumber, 1, 48), "Start-up number"));
  _edit.appendChild(addBlock(generateSwitch('rgb-enable', _rgbEnable, 'Enabled', 'Disabled'), "RGB"));
  

  _edit.appendChild(addBlock(setSaveLed(_index)), '');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });


 }
}




function setSaveLed(_index) {
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveLed(' + _index + ');"><span>Save</span></a>';
}





function sendMessageSaveLed(_index) {
 var _activationNote = getValueRange('activation-note');
 var _startupNumber = getValueRange('startup-number');
 var _rgbEnable = getValueSwitch('rgb-enable');
 
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x04, 0x01, _index, _activationNote, 0xf7]);// Send activation note
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x04, 0x02, _index, _startupNumber, 0xf7]); // Send start-up number
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x04, 0x03, _index, _rgbEnable, 0xf7]); // Send RGB enable

}

