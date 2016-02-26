
//var _programChangeEnable = window.currentData[1];
//var _MidiID = window.currentData[0];
//var _buttonType = window.currentData[2];
//Get all conf data for buttons

var _analogCount = 32;

function sendAnalogGet() {

 //window.currentAction = ENUM_Action.GET_Conf_Buttons;
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x00, 0xf7]); //Enabled state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x01, 0xf7]); //Inverted state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x02, 0xf7]); //Types
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x03, 0xf7]); //MIDI ID
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x04, 0xf7]); //Lower CC limit
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x03, 0x05, 0xf7]); //Upper CC limit
 
 setTimeout(function () {
  fillAllAnalog();
 }, 500);

}


function fillAllAnalog() {

 resetContent();
 for (var i = 0; i < _analogCount; i++) {
  var _a = document.createElement('a');
  _a.setAttribute('href', "javascript:void(0);");
  _a.setAttribute('class', 'buttonOuter');
  _a.setAttribute('onclick', 'editAnalogData(' + i + ');')
  _a.innerHTML += '<span class="buttonInner">' + (i + 1) + '</span>';

  var temp = getDataForIndex(i, window.currentData[3]);
  _a.innerHTML += '<span class="buttonInnerLevel0">MIDI ID: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[0]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';

  _a.innerHTML += '<span class="buttonInnerLevel1">En. state: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[1]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';
  _a.innerHTML += '<span class="buttonInnerLevel2">In. state: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[2]);
  if (temp == 1)
   temp = 'Potentiometer';
  else
   temp = 'FSR';
  _a.innerHTML += '<span class="buttonInnerLevel3">Type: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[4]);
  _a.innerHTML += '<span class="buttonInnerLevel4">Lower-CC: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[5]);
  _a.innerHTML += '<span class="buttonInnerLevel5">Upper-CC: ' + temp + '</span>';
  temp = null;

  document.getElementById('content').appendChild(_a);
 }
}

function editAnalogData(_index) {
 if (window.currentData != undefined) {

  

  var _enabledState = getDataForIndex(_index, window.currentData[0]);
  var _invertedState = getDataForIndex(_index, window.currentData[1]);
  var _types = getDataForIndex(_index, window.currentData[2]);
  var _midiID = getDataForIndex(_index, window.currentData[3]);
  var _lowerCC = getDataForIndex(_index, window.currentData[4]);
  var _upperCC = getDataForIndex(_index, window.currentData[5]);

  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText(_index + 1), "Analog"));
  _edit.appendChild(addBlock(generateRange('midiid', _midiID, 0, 127), "Midi ID"));

  _edit.appendChild(addBlock(generateSwitch('enabled-state', _enabledState, 'Enabled', 'Disabled'), "Enable state"));
  _edit.appendChild(addBlock(generateSwitch('inverted-state', _invertedState, 'Enabled', 'Disabled'), "Inverted state"));
  _edit.appendChild(addBlock(generateSwitch('type', _types, 'FSR', 'Potentiometer'), "Type"));

  _edit.appendChild(addBlock(generateRange('lower-cc', _lowerCC, 0, 127), "Lower CC"));
  _edit.appendChild(addBlock(generateRange('upper-cc', _upperCC, 0, 127), "Upper CC"));

  _edit.appendChild(addBlock(setSaveAnalog(_index)), '');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });


 }
}




function setSaveAnalog(_index) {
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveAnalog(' + _index + ');"><span>Save</span></a>';
}




function sendMessageSaveAnalog(_index) {
 var _enabledState = getValueSwitch('enabled-state');
 var _invertedState = getValueSwitch('inverted-state');
 var _types = getValueSwitch('type');
 var _midiID = getValueRange('midiid');
 var _lowerCC = getValueRange('lower-cc');
 var _upperCC = getValueRange('upper-cc');


 


 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x00, _index, _enabledState, 0xf7]); //Enabled state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x01, _index, _invertedState,0xf7]); //Inverted state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x02, _index, _types, 0xf7]); //Types
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x03, _index, _midiID, 0xf7]); //MIDI ID
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x04, _index, _lowerCC ,0xf7]); //Lower CC limit
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x03, 0x05, _index, _upperCC, 0xf7]); //Upper CC limit

}

