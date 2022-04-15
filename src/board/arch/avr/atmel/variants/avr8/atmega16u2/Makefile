ifeq ($(TYPE),boot)
    #save flash - this feature doesn't fit into 4k
    DEFINES := $(filter-out LED_INDICATORS_CTL,$(DEFINES))
    UART_TX_BUFFER_SIZE := 32
    UART_RX_BUFFER_SIZE := 32
else ifeq ($(TYPE),app)
    UART_TX_BUFFER_SIZE := 64
    UART_RX_BUFFER_SIZE := 64
endif

# some extra optimization to fit bootloader
CPPFLAGS += -flto
LDFLAGS  += -flto
