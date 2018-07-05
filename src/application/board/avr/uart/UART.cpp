#include "board/Board.h"
#include "board/common/indicators/Variables.h"
#include "Constants.h"

///
/// \brief Buffer in which outgoing data is stored.
///
static RingBuff_t   txBuffer;

///
/// \brief Buffer in which incoming data is stored.
///
static RingBuff_t   rxBuffer;

///
/// \brief Flag determining whether or not UART loopback functionality is enabled.
/// When enabled, all incoming UART traffic is immediately passed on to UART TX.
///
static bool         loopbackEnabled;

///
/// \brief Flag determining whether or not OpenDeck UART format is configured.
///
static bool         odUARTconfigured;

///
/// \brief ISR used to store incoming data from UART to buffer.
///
ISR(USART_RX_vect)
{
    uint8_t data = UDR;

    if (!loopbackEnabled)
    {
        if (!RingBuffer_IsFull(&rxBuffer))
        {
            RingBuffer_Insert(&rxBuffer, data);
        }
    }
    else
    {
        if (!RingBuffer_IsFull(&txBuffer))
        {
            RingBuffer_Insert(&txBuffer, data);
            UCSRB |= (1<<UDRIE);
            #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
            MIDIsent = true;
            MIDIreceived = true;
            #endif
        }
}
}

///
/// \brief ISR used to write outgoing data in buffer to UART.
///
ISR(USART_UDRE_vect)
{
    if (RingBuffer_IsEmpty(&txBuffer))
    {
        //buffer is empty, disable transmit interrupt
        UCSRB &= ~(1<<UDRIE);
    }
    else
    {
        uint8_t data = RingBuffer_Remove(&txBuffer);
        UDR = data;
    }
}

void Board::initMIDI_UART()
{
    int32_t baud_count = ((F_CPU / 8) + (MIDI_BAUD_RATE / 2)) / MIDI_BAUD_RATE;

    //clear registers first
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        UCSRA = 0;
        UCSRB = 0;
        UCSRC = 0;
        UBRR = 0;
    }

    if ((baud_count & 1) && baud_count <= 4096)
    {
        UCSRA = (1<<U2X); //double speed uart
        UBRR = baud_count - 1;
    }
    else
    {
        UCSRA = 0;
        UBRR = (baud_count >> 1) - 1;
    }

    //8 bit, no parity, 1 stop bit
    UCSRC = (1<<UCSZ1) | (1<<UCSZ0);

    //enable receiver, transmitter and receive interrupt
    UCSRB = (1<<RXEN) | (1<<TXEN) | (1<<RXCIE);

    RingBuffer_InitBuffer(&rxBuffer);
    RingBuffer_InitBuffer(&txBuffer);

    #ifndef BOARD_A_xu2
    #if defined(BOARD_A_MEGA) || defined(BOARD_A_UNO)
    //enable od format immediately for these boards
    setOD_UART();
    #else
    midi.handleUARTread(board.MIDIread_UART);
    midi.handleUARTwrite(board.MIDIwrite_UART);
    #endif
    #endif
}

int16_t Board::MIDIread_UART()
{
    if (RingBuffer_IsEmpty(&rxBuffer))
    {
        return -1;
    }

    uint8_t data = RingBuffer_Remove(&rxBuffer);
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO) && !defined(BOARD_A_xu2)
    MIDIreceived = true;
    #endif
    return data;
}

int8_t Board::MIDIwrite_UART(uint8_t data)
{
    //if both the outgoing buffer and the UART data register are empty
    //write the byte to the data register directly
    if (RingBuffer_IsEmpty(&txBuffer) && (UCSRA & (1<<UDRE)))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UDR = data;
        }

        #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
        MIDIsent = true;
        #endif

        return 1;
    }

    while (RingBuffer_IsFull(&txBuffer));
    RingBuffer_Insert(&txBuffer, data);
    UCSRB |= (1<<UDRIE);
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
    MIDIsent = true;
    #endif

    return 1;
}

bool Board::MIDIwrite_UART_OD(USBMIDIpacket_t& USBMIDIpacket)
{
    RingBuffer_Insert(&txBuffer, 0xF1);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Event);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data1);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data2);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Data3);
    RingBuffer_Insert(&txBuffer, USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

    UCSRB |= (1<<UDRIE);
    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO) && !defined(BOARD_A_xu2)
    MIDIsent = true;
    #endif

    return true;
}

bool Board::MIDIread_UART_OD()
{
    if (RingBuffer_GetCount(&rxBuffer) >= 6)
    {
        int16_t data = MIDIread_UART();

        if (data == 0xF1)
        {
            //start of frame, read rest of the packet
            for (int i=0; i<5; i++)
            {
                data = MIDIread_UART();

                switch(i)
                {
                    case 0:
                    usbMIDIpacket.Event = data;
                    break;

                    case 1:
                    usbMIDIpacket.Data1 = data;
                    break;

                    case 2:
                    usbMIDIpacket.Data2 = data;
                    break;

                    case 3:
                    usbMIDIpacket.Data3 = data;
                    break;

                    case 4:
                    //xor byte, do nothing
                    break;
                }
            }

            //everything fine so far
            #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO) && !defined(BOARD_A_xu2)
            MIDIreceived = true;
            #endif

            //error check
            uint8_t dataXOR = usbMIDIpacket.Event ^ usbMIDIpacket.Data1 ^ usbMIDIpacket.Data2 ^ usbMIDIpacket.Data3;

            return (dataXOR == data);
        }
    }

    return false;
}

bool usbRead_od(USBMIDIpacket_t& USBMIDIpacket)
{
    return board.MIDIread_UART_OD();
}

void Board::setUARTloopbackState(bool state)
{
    loopbackEnabled = state;
}

bool Board::getUARTloopbackState()
{
    return loopbackEnabled;
}

bool Board::isRXempty()
{
    return RingBuffer_IsEmpty(&rxBuffer);
}

bool Board::isTXempty()
{
    return RingBuffer_IsEmpty(&txBuffer);
}

void Board::setOD_UART()
{
    #ifdef BOARD_OPEN_DECK
    if (isUSBconnected())
    {
        //master board
        midi.handleUSBread(board.MIDIread_USB_write_UART_OD);
        midi.handleUSBwrite(board.MIDIwrite_USB);
        midi.handleUARTread(NULL); //parsed internally
        midi.handleUARTwrite(NULL);
    }
    else
    {
        //slave
        midi.handleUSBread(usbRead_od); //loopback used on inner slaves
        midi.handleUSBwrite(board.MIDIwrite_UART_OD);
        midi.handleUARTread(NULL);
        midi.handleUARTwrite(NULL);
    }

    odUARTconfigured = true;
    #elif defined(BOARD_A_MEGA) || defined(BOARD_A_UNO)
    midi.handleUSBread(usbRead_od);
    midi.handleUSBwrite(board.MIDIwrite_UART_OD);
    midi.handleUARTread(NULL);
    midi.handleUARTwrite(NULL);
    odUARTconfigured = true;
    #endif
}

bool Board::getOD_UART()
{
    return odUARTconfigured;
}
