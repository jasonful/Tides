#include "restartcounter.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

RestartCounter::RestartCounter()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );
}

RestartCounter::~RestartCounter()
{
  nvs_flash_deinit();
}


int32_t RestartCounter::Get (void)
{
    int32_t restart_counter = 1; // value will default to 1if not set yet in NVS

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Read
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
        switch (err) {
            case ESP_OK:
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
        }

        // Just in case something goes wrong...
        if (restart_counter < 0 || restart_counter > 1000) {
          restart_counter = 1;
        }

        // Close
        nvs_close(my_handle);
    }
    return restart_counter;
}


// Store a value in nonvolatile storage indicating how many TPL5110-induced
// wakeups should happen before we fetch new data.
void RestartCounter::Set (int32_t restart_counter)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        err = nvs_commit(my_handle);

        // Close
        nvs_close(my_handle);
    }
}

