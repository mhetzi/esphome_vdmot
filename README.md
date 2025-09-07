<a href="https://esphome.io/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./logo-text-on-dark.svg", alt="ESPHome Logo">
    <img src="./logo-text-on-light.svg" alt="ESPHome Logo">
  </picture>
</a>

## HmIP-VDMOT component
<p> This external compoment allowes the use of valves, with a current endstop. </p>
<p> 
IF the current of the motor crosses the configured threshold. Stop the output.
Learns the time it takes for a complete open to close cycle and uses that for Percentage based open.
</p>
<p>
The necessary section of the device YAML not everything is needed
</p>

```
# Pin configuration for both directions
output:  
  - platform: gpio
    pin: 13
    id: 'pin_valve_open'
    inverted: false
  - platform: gpio
    pin: 14
    id: 'pin_valve_close'
    
# The main component
vdmot:
  ina219_id: ina_component
  output_close_id: pin_valve_close
  output_open_id: pin_valve_open
  ina219_current_id: ina_current
  max_motor_current: 0.06
  id: vdmot_hubb
  # The valve component to interact
  valve:
    name: Fußbodenheizung Ventil
    id: valve_1
  # Relearn the open close cycle
  calibrate:
    name: "Kalibriere Ventil"

# This is the current sensor, we use as input
sensor:
  - platform: ina219
    address: 0x40
    id: ina_component
    shunt_resistance: 0.1 ohm
    current:
      name: "INA219 Current"
      id: ina_current
      filters:
        - delta: 0.00050
    max_voltage: 3.3V
    max_current: 0.4A
    update_interval: never #While driving the Ventil Motor, this gets updated continuously.
```

## Going further

- [esphome Developer documentation](https://developers.esphome.io)
- [esphome Component architecture overview](https://developers.esphome.io/architecture/components/)
