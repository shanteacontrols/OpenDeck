ifeq ($(TYPE),boot)
    UART_TX_BUFFER_SIZE := 32
    UART_RX_BUFFER_SIZE := 32
else ifeq ($(TYPE),app)
    UART_TX_BUFFER_SIZE := 64
    UART_RX_BUFFER_SIZE := 64
endif

# Some extra optimization to fit bootloader
CPPFLAGS += -flto
LDFLAGS  += -flto
