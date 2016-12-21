#include "Board.h"
#include "Variables.h"
#include "Constants.h"

bool                dmBufferCopied = false;
uint64_t            inputMatrixBufferCopy;

volatile uint64_t   inputBuffer[DIGITAL_BUFFER_SIZE];
volatile uint8_t    digital_buffer_head = 0;
volatile uint8_t    digital_buffer_tail = 0;
volatile uint8_t    activeButtonColumn  = 0;

int8_t getInputMatrixBufferSize()
{
    uint8_t head, tail;

    head = digital_buffer_head;
    tail = digital_buffer_tail;
    if (head >= tail)
    return head - tail;

    return DIGITAL_BUFFER_SIZE + head - tail;
}

bool Board::copyInputMatrixBuffer()
{
    int8_t bufferSize = getInputMatrixBufferSize();

    if (bufferSize <= 0)
    return false;

    //some data in buffer
    //copy oldest member of buffer to inputMatrixBufferCopy
    if (digital_buffer_head == digital_buffer_tail)
    return false;

    uint8_t index = digital_buffer_tail + 1;
    if (index >= DIGITAL_BUFFER_SIZE)
    index = 0;

    cli();
    inputMatrixBufferCopy = inputBuffer[index];
    sei();

    dmBufferCopied = true;
    buttonsProcessed = false;
    encodersProcessed = false;
    digital_buffer_tail = index;

    return true;
}

void Board::checkInputMatrixBufferCopy()
{
    if ((buttonsProcessed == true) && (encodersProcessed == true) && (dmBufferCopied == true))
    {
        dmBufferCopied = false;
        buttonsProcessed = false;
        encodersProcessed = false;
    }
}