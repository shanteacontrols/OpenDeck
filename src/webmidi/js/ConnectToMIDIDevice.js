
var midi, data;

function checkForDevices() {
    // request MIDI access
    if (navigator.requestMIDIAccess) {
        navigator.requestMIDIAccess({
            sysex: true // this defaults to 'false' and we won't be covering sysex in this article. 
        }).then(onMIDISuccess, onMIDIFailure);
    } else {
        alert("No MIDI support in your browser.");
    }
}

// midi functions
function onMIDISuccess(midiAccess) {
 // when we get a succesful response, run this code

 //console.log('MIDI Access Object', midiAccess);

 midi = midiAccess; // this is our raw MIDI data, inputs, outputs, and sysex status
 //window.midiAccess = midi;
 setupMidiDevices();
 midi.onstatechange = onStateChange;
}


function setupMidiDevices()
{
 var inputs = midi.inputs.values();
 // loop over all available inputs and listen for any MIDI input
 var count = 0;
 var id;

 for (var input = inputs.next() ; input && !input.done; input = inputs.next()) {
  if (input.value.name.indexOf('OpenDeck') != -1) {
   id = input.value.id;
   count += 1;
   // each time there is a midi message call the onMIDIMessage function
   input.value.onmidimessage = onMIDIMessage;

   listInputs(input);
  }
 }


 var outputs = midi.outputs.values();

 for (var output = outputs.next() ; output && !output.done; output = outputs.next()) {
  if (output.value.name.indexOf('OpenDeck') != -1) {
   output.value.onmidimessage = onMIDIMessage;

  }
 }


 if (window.activeDevice == undefined) {
  if (count == 1) {
   window.activeDevice = midi.inputs.get(id);
   setHelloMessage();
   deviceFounded();
  } else {
   if (count != 0) {
    $('#ripple-content').html('Multiple devices detected. Please connect only one OpenDeck!');
   }
  }
 }
}

function onStateChange(event) {
 //if (event.srcElement.inputs.size == 0) {

 //}
 setTimeout(function () {
  if (event.port.state == 'disconnected' && event.port.name == 'OpenDeck') {
   window.activeDevice = undefined;
   $('#ripple-content').html('Searching for device');
   $('#main').fadeOut(400, function () {
    $('#loader').fadeIn(400);
    $('.flip-container').removeClass('hover');
   });
  }

  if (window.activeDevice == undefined) {
   setTimeout(function () {
    setupMidiDevices();
   }, 1000);
  }
 }, 500);
}


function onSingleInputStateChange(event) {
 var i;
}

function onSingleOutputStateChange(event) {
 var i;
}

function showChoice() {

}

function deviceFounded() {
 flip();
 setTimeout(function () {
  $('#ripple-content').html('Device found!');
  setTimeout(function () {
   $('#loader').fadeOut(1000, function () {
    $('#main').fadeIn();
    
   });
  }, 500);
 }, 500);
}

function setHelloMessage() {
 sendHelloMessage();
 setInterval(function () { sendHelloMessage(); }, 30000);
}

function listInputs(inputs) {
    var input = inputs.value;
    //device.textContent = input.name.replace(/port.*/i, '');
    console.log("Input port : [ type:'" + input.type + "' id: '" + input.id +
            "' manufacturer: '" + input.manufacturer + "' name: '" + input.name +
            "' version: '" + input.version + "']");
}



function onMIDIFailure(e) {
    // when we get a failed response, run this code
    console.log("No access to MIDI devices or your browser doesn't support WebMIDI API. Please use WebMIDIAPIShim " + e);
}


function onMIDIMessage(message) {
 if (window.skipFill) { window.skipFill = !window.skipFill; return; }
 data = message.data;
 if (window.currentData == undefined) { window.currentData = new Array; };
 window.currentData.push(data);
 //console.log('OnMidiMessage', 'Current action:' + window.currentAction);
 //console.log('OnMidiMessageData', data);
}

function sendMessage(message) {
 if (window.activeDevice == undefined) { return; }
 var output = midi.outputs.get(window.activeDevice.id);
 if (output)
    output.send(message);   
}