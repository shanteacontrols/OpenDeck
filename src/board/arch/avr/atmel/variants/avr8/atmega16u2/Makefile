ifeq ($(TYPE),bootloader)
    BUFFER_SIZE_UART_TX := 32
    BUFFER_SIZE_UART_RX := 32
else ifeq ($(TYPE),application)
    BUFFER_SIZE_UART_TX := 64
    BUFFER_SIZE_UART_RX := 64
endif

CPPFLAGS += -flto
LDFLAGS  += -flto