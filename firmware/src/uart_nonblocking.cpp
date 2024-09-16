#include <uart_nonblocking.h>


void __not_in_flash_func(dispatch_uart_stream)(uint dma_chan, uart_inst_t* uart,
                                               uint8_t* starting_address,
                                               size_t word_count)
{
    // DMA channel will write data to the uart, paced by DREQ_TX.
    dma_channel_config conf = dma_channel_get_default_config(dma_chan);

    // Setup Sample Channel.
    channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    channel_config_set_read_increment(&conf, true); // read from starting memory address.
    channel_config_set_write_increment(&conf, false); // write to fixed uart memory address.
    channel_config_set_irq_quiet(&conf, true);
    // Pace data according to pio providing data.
    uint uart_dreq = (uart == uart0)? DREQ_UART0_TX : DREQ_UART1_TX;
    channel_config_set_dreq(&conf, uart_dreq);
    channel_config_set_enable(&conf, true);
    // Apply samp_chan_ configuration.
    dma_channel_configure(
        dma_chan,               // Channel to be configured
        &conf,                  // corresponding DMA config.
        &uart_get_hw(uart)->dr, // write (dst) address.
        starting_address,       // read (source) address.
        word_count,             // Number of word transfers i.e: len(string).
        true                    // Do start immediately.
    );
}
