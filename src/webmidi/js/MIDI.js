/// <reference path="MIDI.js" />

//var _programChangeEnable = window.currentData[1];
//var _MidiID = window.currentData[0];
//var _buttonType = window.currentData[2];
//Get all conf data for buttons



function sendMIDIGet() {

 //window.currentAction = ENUM_Action.GET_Conf_Buttons;
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x00, 0x00, 0xf7]); //standard note off, running status, usb convert
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x00, 0x01, 0xf7]); //note channel, program change channel, cc channel, input channel
 
 setTimeout(function () {
  editMIDIData();
 }, 500);
 
}

function editMIDIData() {
 if (window.currentData != undefined) {
  //features
  var _standardNote = getDataForIndex(0, window.currentData[0]);
  var _runningStatus = getDataForIndex(1, window.currentData[0]);
  var _usbConvert = getDataForIndex(2, window.currentData[0]);
  //channels
  var _noteChannel = getDataForIndex(0, window.currentData[1]);
  var _programChangeChannel = getDataForIndex(1, window.currentData[1]);
  var _ccChannel = getDataForIndex(2, window.currentData[1]);
  var _inputChannel = getDataForIndex(3, window.currentData[1]);

  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText('Global setup'), "MIDI - features"));
  _edit.appendChild(addBlock(generateSwitch('standard-note', _standardNote, 'Enabled', 'Disabled'), "Standard note"));
  _edit.appendChild(addBlock(generateSwitch('running-status', _runningStatus, 'Enabled', 'Disabled'), "Running status"));
  _edit.appendChild(addBlock(generateSwitch('usb', _usbConvert, 'Enabled', 'Disabled'), "Usb convert"));

  _edit.appendChild(addBlock(generateText('Global setup'), "MIDI - channels"));
  _edit.appendChild(addBlock(generateRange('note-channel', _noteChannel, 1, 16), "Note channel"));
  _edit.appendChild(addBlock(generateRange('program-change', _programChangeChannel, 1, 16), "Program change channel"));
  _edit.appendChild(addBlock(generateRange('cc-channel', _ccChannel, 1, 16), "CC channel"));
  _edit.appendChild(addBlock(generateRange('input-channel', _inputChannel, 1, 16), "Input channel"));

  _edit.appendChild(addBlock(setSaveMIDI()), '');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });


 }
}


function setSaveMIDI() {
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveMIDI();"><span>Save</span></a>';
}




function sendMessageSaveMIDI() {
 

 //features
 var _standardNote = getValueSwitch('standard-note');
 var _runningStatus = getValueSwitch('running-status');
 var _usbConvert = getValueSwitch('usb');
 //channels
 var _noteChannel = getValueRange('note-channel');
 var _programChangeChannel = getValueRange('program-change');
 var _ccChannel = getValueRange('cc-channel');
 var _inputChannel = getValueRange('input-channel');
 
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x01, 0x00, 0x00, _standardNote, _runningStatus, _usbConvert , 0xf7]); //Features

 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x01, 0x00, 0x01, _noteChannel, _programChangeChannel, _ccChannel, _inputChannel, 0xf7]); //Channels
 

}
