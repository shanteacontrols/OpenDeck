var _encoderCount = 32;

function sendEncoderGet() {

 //window.currentAction = ENUM_Action.GET_Conf_Buttons;
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x02, 0x03, 0xf7]); //MIDI ID get message
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x02, 0x00, 0xf7]); //enabled states
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x02, 0x01, 0xf7]); //inverted states
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x02, 0x02, 0xf7]); //enocoding modes
 setTimeout(function () {
  fillAllEncoders();
 }, 500);

}


function fillAllEncoders() {

 resetContent();
 for (var i = 0; i < _encoderCount; i++) {
  var _a = document.createElement('a');
  _a.setAttribute('href', "javascript:void(0);");
  _a.setAttribute('class', 'buttonOuter');
  _a.setAttribute('onclick', 'editEncoderData(' + i + ');')
  _a.innerHTML += '<span class="buttonInner">' + (i + 1) + '</span>';

  var temp = getDataForIndex(i, window.currentData[0]);
  _a.innerHTML += '<span class="buttonInnerLevel0">MIDI ID: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[1]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';

  _a.innerHTML += '<span class="buttonInnerLevel1">En. state: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[2]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';
  _a.innerHTML += '<span class="buttonInnerLevel2">In. state: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[3]);
  if (temp == 1)
   temp = '7Fh01h';
  else
   temp = '3Fh41h';
  _a.innerHTML += '<span class="buttonInnerLevel3">Encoding: ' + temp + '</span>';
  temp = null;

  document.getElementById('content').appendChild(_a);
 }
}



function editEncoderData(_index) {
 if (window.currentData != undefined) {

  var _midiID = getDataForIndex(_index, window.currentData[0]);
  var _enabledState = getDataForIndex(_index, window.currentData[1]);
  var _invertedState = getDataForIndex(_index, window.currentData[2]);
  var _encodingMode = getDataForIndex(_index, window.currentData[3]);

  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText(_index + 1), "Encoder"));
  _edit.appendChild(addBlock(generateRange('midiid', _midiID, 0, 127), "Midi ID"));

  _edit.appendChild(addBlock(generateSwitch('enabled-state', _enabledState, 'Enabled', 'Disabled'), "Enabled state"));
  _edit.appendChild(addBlock(generateSwitch('inverted-state', _invertedState, 'Enabled', 'Disabled'), "Inverted state"));
  _edit.appendChild(addBlock(generateSwitch('encoding-mode', _encodingMode, '7Fh01h', '3Fh41h'), "Encoding mode"));

  _edit.appendChild(addBlock(setSaveEncoder(_index)), '');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });


 }
}




function setSaveEncoder(_index) {
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveEncoder(' + _index + ');"><span>Save</span></a>';
}




function sendMessageSaveEncoder(_index) {
 var _midiID = getValueRange('midiid');
 var _enabledStates = getValueSwitch('enabled-state');
 var _invertedStates = getValueSwitch('inverted-state');
 var _encodingMode = getValueSwitch('encoding-mode');

 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x02, 0x03, _index, _midiID, 0xf7]); //Send MIDI ID
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x02, 0x00, _index, _enabledStates, 0xf7]);// Send enabled state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x02, 0x01, _index, _invertedStates, 0xf7]); // Send inverted state
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x02, 0x02, _index, _encodingMode, 0xf7]); // Send encoding mode

}

