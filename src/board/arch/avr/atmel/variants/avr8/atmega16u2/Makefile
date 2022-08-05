ifeq ($(TYPE),boot)
    BUFFER_SIZE_UART_TX := 32
    BUFFER_SIZE_UART_RX := 32
else ifeq ($(TYPE),app)
    BUFFER_SIZE_UART_TX := 64
    BUFFER_SIZE_UART_RX := 64
endif

# Some extra optimization to fit bootloader
CPPFLAGS += -flto
LDFLAGS  += -flto
