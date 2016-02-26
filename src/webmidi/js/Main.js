var ENUM_Action = {
 GET_Conf_Buttons: 0
};



function resetData() {
 window.currentData = undefined;
}

function resetContent()
{
 document.getElementById('content').innerHTML = '';
}

function fillData(content)
{
    document.getElementById('content').innerHTML=content;
}

function sendHelloMessage()
{
 window.skipFill = true;
 sendMessage([0xf0, 0x00, 0x53, 0x43, 0x48, 0xf7]);
}


function generateSwitch(_id, _selected, _valueOn, _valueOff) {
 var _checked = 'checked';
 if (_selected == 0) { _checked = ''; }
 var out = '';
 out += '<div class="switch">';
 out += '<input id="' + _id + '" class="cmn-toggle cmn-toggle-yes-no" type="checkbox" ' + _checked + '>';
 out += '<label for="'+ _id + '" data-on="' + _valueOn + '" data-off="' + _valueOff + '"></label>';
 out += '</div>';
 return out;
}

function generateRange(_id, _selected, _min, _max, _multiply) {
 var _val = _selected;
 if (!isNaN(_multiply)) { _val *= _multiply; }

 return '<label class="range_label">' + _val + '</label><div class="clear"></div><input id="' + _id + '" type="range" class="range" value="' + _selected + '" min="' + _min + '" max="' + _max + '" oninput="updateValue(this, ' + _multiply + ');" />'
}


function generateText(_value) {
 return '<label class="edit_text">' + _value + '</label>';
}

function updateValue(sender, _multiply) {
 var val = sender.value;
 if (!isNaN(_multiply)) { val = val * _multiply; }
 $(sender).parent().find('label').html(val);
}

function getValueRange(_id) {
 return $('#' + _id).val();
}

function getValueSwitch(_id) {
 var _checked = $('#' + _id).prop('checked');
 if (_checked) return 1;
 return 0;
}


function addBlock(_value, _header) {
 var _block = document.createElement('div');
 _block.setAttribute('class', 'block');
 if (_header)
  _block.innerHTML = '<h3 class="header">' + _header + '</h3>' + _value;
 else
  _block.innerHTML = _value;
 return _block;
}


function getDataForIndex(_index, _array) {
 return _array[_index + getZeroIndex(_array)];
}


function getZeroIndex(_array) {
 return _array.indexOf(65) + 1;
}

function flip() {
 $('.flip-container').toggleClass('hover');
}

function showContent() {
 $('#content').css('display', 'block');
 $('#edit').css('display', 'none');
}