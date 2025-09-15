idf_component_register(SRCS "Estufa_Rogerio1.c" "RMenus.c" "RMemoria_NVS.c" "RMenus.c"
                    INCLUDE_DIRS "."
                    PRIV_INCLUDE_DIRS "../components/Componente_Rele"
                    PRIV_INCLUDE_DIRS "../components/Componente_LAB01_Sensores"
                    # REQUIRES "Componente_LAB01_Sensores" "Componente_Rele" "driver" "esp_adc" "nvs_flash" "esp_timer" "freertos" "ssd1306" "i2cdev" "ds3231" "esp_log"
)