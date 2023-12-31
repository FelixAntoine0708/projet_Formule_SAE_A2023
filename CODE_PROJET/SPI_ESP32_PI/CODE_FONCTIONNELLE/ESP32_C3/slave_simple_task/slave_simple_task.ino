/*
brief : Ce code est un des multiples exemples que Arduino IDE a à offrir. Il permet au ESP32-C3 de communiquer avec le Pi 3B en SPI.
        J'ai modifié le code pour qu'il envoie les trois caractères suivant : H, L, O. Le ESP32-C3 est le SLAVE.
*/



#include <ESP32SPISlave.h>

ESP32SPISlave slave;

static constexpr uint32_t BUFFER_SIZE {32};
uint8_t spi_slave_tx_buf[BUFFER_SIZE];
uint8_t spi_slave_rx_buf[BUFFER_SIZE];
uint8_t tx_buf[BUFFER_SIZE] = {'H', 'L', 'O'};      // MA MODIFICATION QUE J'AI FAIT


void set_buffer() {
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        spi_slave_tx_buf[i] = (0xFF - i) & 0xFF;
    }
    //memset(spi_slave_rx_buf, 0, BUFFER_SIZE);
    memset(spi_slave_rx_buf, 0, BUFFER_SIZE);   // MODIFICATION QUE J'AI FAIT
}

constexpr uint8_t CORE_TASK_SPI_SLAVE {0};
constexpr uint8_t CORE_TASK_PROCESS_BUFFER {0};

static TaskHandle_t task_handle_wait_spi = 0;
static TaskHandle_t task_handle_process_buffer = 0;

void task_wait_spi(void* pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // block until the transaction comes from master
        //slave.wait(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);
        slave.wait(spi_slave_rx_buf, tx_buf, BUFFER_SIZE);  // MODIFICATION QUE J'AI FAIT
        xTaskNotifyGive(task_handle_process_buffer);
    }
}

void task_process_buffer(void* pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // show received data
        for (size_t i = 0; i < BUFFER_SIZE; ++i) {
            printf("%d ", spi_slave_rx_buf[i]);
        }
        printf("\n");

        slave.pop();

        xTaskNotifyGive(task_handle_wait_spi);
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    // begin() after setting
    // note: the default pins are different depending on the board
    // please refer to README Section "SPI Buses and SPI Pins" for more details
    slave.setDataMode(SPI_MODE0);
    slave.begin(); // HSPI
    // slave.begin(VSPI);   // you can use VSPI like this

    set_buffer();

    xTaskCreatePinnedToCore(task_wait_spi, "task_wait_spi", 2048, NULL, 2, &task_handle_wait_spi, CORE_TASK_SPI_SLAVE);
    xTaskNotifyGive(task_handle_wait_spi);

    xTaskCreatePinnedToCore(
        task_process_buffer,
        "task_process_buffer",
        2048,
        NULL,
        2,
        &task_handle_process_buffer,
        CORE_TASK_PROCESS_BUFFER);
}

void loop() {}
