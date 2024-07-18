#include <hardware/dma.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>
#include <cstdint>

/**
 * \brief nonblocking way to dispatch uart characters.
 * \details assumes a global DMA channel has already been assigned.
 */
void __time_critical_func(dispatch_uart_stream)(uint dma_chan,
                                                uart_inst_t* uart,
                                                uint8_t* starting_address,
                                                size_t word_count);
