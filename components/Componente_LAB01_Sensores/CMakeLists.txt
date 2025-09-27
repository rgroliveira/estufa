idf_component_register(SRCS "Componente_LAB01_Sensores.c"
                    INCLUDE_DIRS "."
                    INCLUDE_DIRS "include"
                    REQUIRES "driver" "esp_adc"
                    PRIV_INCLUDE_DIRS "../dht" 
                    )
