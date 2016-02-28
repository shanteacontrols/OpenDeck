
//var _programChangeEnable = window.currentData[1];
//var _MidiID = window.currentData[0];
//var _buttonType = window.currentData[2];
//Get all conf data for buttons

var _buttonCount = 64;

function sendButtonGet()
{

 //window.currentAction = ENUM_Action.GET_Conf_Buttons;
 resetData();
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x01, 0x02, 0xf7]); //MIDI ID get message
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x01, 0x01, 0xf7]); //Program change enable get message
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x00, 0x01, 0x01, 0x00, 0xf7]); //Button type get message
 setTimeout(function () {
  fillAllButtons();
 }, 500);
 
}


function fillAllButtons()
  {
 
 resetContent();

 for (var i = 0; i < _buttonCount; i++) {
  var _a = document.createElement('a');
  _a.setAttribute('href', "javascript:void(0);");
  _a.setAttribute('class', 'buttonOuter');
  _a.setAttribute('onclick', 'editButtonData(' + i + ');')
  _a.innerHTML += '<span class="buttonInner">' + (i + 1) + '</span>';

  var temp = getDataForIndex(i, window.currentData[0]);
  _a.innerHTML += '<span class="buttonInnerLevel0">MIDI ID: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[1]);
  if (temp == 1)
   temp = 'Enabled';
  else
   temp = 'Disabled';

  _a.innerHTML += '<span class="buttonInnerLevel1">Program state: ' + temp + '</span>';
  temp = null;

  temp = getDataForIndex(i, window.currentData[2]);
  if (temp == 1)
   temp = 'Latching';
  else
   temp = 'Momentary';
  _a.innerHTML += '<span class="buttonInnerLevel2">Type: ' + temp + '</span>';
  temp = null;

  document.getElementById('content').appendChild(_a);
 }
}

function editButtonData(_index)
{
 if (window.currentData != undefined) {
  
  var _m = getDataForIndex(_index, window.currentData[0]);
  var _p = getDataForIndex(_index, window.currentData[1]);
  var _b = getDataForIndex(_index, window.currentData[2]);

  var _edit = document.getElementById('edit');
  _edit.innerHTML = '';

  _edit.appendChild(addBlock(generateText(_index + 1 ), "Button"));
  _edit.appendChild(addBlock(generateRange('midiid', _m, 0, 127), "Midi ID"));
  _edit.appendChild(addBlock(generateSwitch('programstate',_p,'Enabled','Disabled'), "Program state"));
  _edit.appendChild(addBlock(generateSwitch('buttontype', _p, 'Latching', 'Momentary'), "Button type"));

  _edit.appendChild(addBlock(setSaveButton(_index)),'');

  $('#content').fadeOut(100, function () { $('#edit').fadeIn(); });

  
 }
}




function setSaveButton(_index)
{
 return '<a href="javascript:void(0);" class="commandButton" onclick="sendMessageSaveButton(' + _index + ');"><span>Save</span></a>';
}




 function sendMessageSaveButton(_index)
 {
  var _midiID = getValueRange('midiid');
  var _programState = getValueSwitch('programstate');
  var _buttonType = getValueSwitch('buttontype');

  
  sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x01, 0x02, _index, _midiID, 0xf7]); //Send MIDI ID
  sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x01, 0x01, _index, _programState, 0xf7]);// Send program state
  sendMessage([0xf0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x01, 0x00, _index, _buttonType, 0xf7]); // Send button type
  
 }

 