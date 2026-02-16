#include "driver/gpio.h"     // GPIO driver functions
#include "esp_timer.h"       // High resolution software timers
#include "esp_log.h"         // Logging macros

#define BUTTON_GPIO 18   // GPIO pin where button is connected
#define DEBOUNCE_US 30000        // Debounce delay in microseconds (30 ms)

static const char *TAG = "button";   // Tag used for log messages

// Handle to the debounce timer so we can start/stop it 
static esp_timer_handle_t debounce_timer;


// Timer callback function — runs AFTER debounce delay expires 
static void debounce_timer_cb(void *arg) {
    // Read current logic level of the button pin
    int level = gpio_get_level(BUTTON_GPIO);

    // Check if button is pressed (assuming active LOW wiring)
    if (level == 0) {
        // Log a message indicating a valid debounced press
        ESP_LOGI(TAG, "Button pressed (debounced)");
    }
}


// Interrupt Service Routine — runs when GPIO interrupt fires // Must be short and placed in IRAM
 static void IRAM_ATTR button_isr(void *arg) {   
// Stop timer in case it was already running (due to bouncing)
    esp_timer_stop(debounce_timer);

    // Start timer as a one-shot — will call callback after debounce period
   
    esp_timer_start_once(debounce_timer, DEBOUNCE_US); }


void app_main(void)
{
    // Configure GPIO settings structure
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,   // Select button pin
        .mode = GPIO_MODE_INPUT,               // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,      // Enable internal pull-up resistor
        .intr_type = GPIO_INTR_ANYEDGE,        // Trigger interrupt on press AND release
    };

    // Apply configuration to GPIO hardware
    gpio_config(&io_conf);


    // Configure timer parameters
    esp_timer_create_args_t timer_args = {
        .callback = debounce_timer_cb,   // Function to call when timer expires
        .name = "debounce"               // Optional name for debugging
    };

    // Create the software timer
    esp_timer_create(&timer_args, &debounce_timer);

    // Install ISR service (allocates interrupt handling infrastructure)
    gpio_install_isr_service(0);

    // Attach our ISR handler to the button pin
    gpio_isr_handler_add(BUTTON_GPIO, button_isr, NULL); } 
