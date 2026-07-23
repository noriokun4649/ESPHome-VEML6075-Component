# ESPHome VEML6075 component
Integration for VEML6075

## ESPHome Config
(the commented-out raw values are for debugging)
```
external_components:
  - source: github://noriokun4649/ESPHome-VEML6075-Component

sensor:
  - platform: veml6075
    uva:
      name: "UVA"
    uvb:
      name: "UVB"
    uv_index:
      name: "UV Index"
    #raw1:
    #  name: "raw1 Test (UVA)"
    #raw2:
    #  name: "raw2 Test (UVB)"
    #raw3:
    #  name: "raw3 Test (Comp1)"
    #raw4:
    #  name: "raw4 Test (Comp2)"
    integration_time: 100ms
    high_dynamic: false
    update_interval: 20s
```
