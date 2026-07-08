# GridMind Component Inventory

Last updated: 8 July 2026

This inventory records components visually identified from the student's kit. Counts marked **approximate** must be confirmed by laying out and counting the physical parts. Do not connect a component solely from its appearance; verify its pins and electrical requirements first.

**Inventory status:** complete for the OLED-independent MVP. Exact counts of abundant duplicate parts are unnecessary. Pending ordered equipment and backup stock selected for purchase are listed separately and must not be treated as physically available until delivery is confirmed.

Photo evidence inspected:

- `evidence/day2/inventory/components-overview.jpeg`
- `evidence/day2/inventory/components-closeup.jpeg`
- `evidence/day2/inventory/resistors-1.png`
- `evidence/day2/inventory/resistors-2.png`
- `evidence/day2/inventory/resistors-3.png`
- `evidence/day2/inventory/resistor-values.jpeg`
- `evidence/day2/inventory/ldr.jpeg`
- `evidence/day2/inventory/breadboards.png`
- `evidence/day2/inventory/rfid-tags.png`
- `evidence/day2/inventory/buzzer.png`

## Visually confirmed components

### Pushbuttons

- **Quantity seen:** at least 4 larger switches with yellow centres, plus approximately 6 smaller black switches.
- **Appearance:** small square black switches, normally with four metal legs. Some have a coloured or yellow cap in the centre.
- **How they work:** pressing the centre temporarily connects two sides of the switch. The four legs form two internally connected pairs; they are not four independent contacts.
- **GridMind use:** one Scenario/Next button on Node A and Run, Defer and Reduce buttons on Node B.
- **Handling note:** the correct opposing legs must be identified before breadboard wiring. The planned circuit connects a GPIO to ground and uses Arduino's `INPUT_PULLUP` mode.

### Ordinary LEDs

- **Quantity seen:** many; exact count not yet confirmed.
- **Colours seen:** red, yellow, green and blue.
- **Appearance:** small coloured or clear domes with two metal legs.
- **How to identify the legs:** the longer leg is normally the positive leg (anode). The shorter leg and the flat edge of the body normally indicate the negative leg (cathode).
- **GridMind use:** grid-condition indicators on Node A and decision feedback on Node B.
- **Handling note:** every LED must have its own 220-330 ohm series resistor. Never connect an LED directly between a GPIO and ground.

### RGB LED

- **Quantity seen:** 1.
- **Backup stock selected for purchase:** 100 common-cathode, four-pin, 5 mm diffused RGB LEDs, Amazon.fr ASIN `B082X4ZRXJ`; purchase/delivery not yet confirmed.
- **Appearance:** clear LED-style body with four metal legs.
- **How it works:** it contains separate red, green and blue LED channels plus one shared connection. The shared connection may be a common anode or common cathode.
- **Possible GridMind use:** multi-colour decision feedback on Node B.
- **Handling note:** do not assume the pin order or common type. Identify it before connection, and use a separate 220-330 ohm resistor for each colour channel. Keep this single component protected until it is needed.
- **Backup handling note:** the selected backup type is common-cathode, but its exact pin order must still be checked before wiring. It needs three GPIO channels for independent red, green and blue control, so do not substitute it into the pin map without review.

### DHT11 temperature and humidity sensor

- **Quantity seen:** 1.
- **Appearance:** blue rectangular plastic body with ventilation slots and four legs.
- **What it measures:** approximate ambient temperature and relative humidity.
- **Possible GridMind use:** optional ambient-temperature proxy only.
- **Scope note:** room temperature is not data-centre temperature. The current minimum viable project keeps temperature and cooling values scenario-generated, so this sensor is not required.

### Thermistor

- **Quantity seen:** 1 probable component, marked `103`.
- **Appearance:** small orange/brown disc with two legs.
- **What it does:** its resistance changes with temperature. The `103` marking commonly indicates a nominal resistance of 10 kOhm, but this must be confirmed before use.
- **GridMind use:** none currently planned.

### Slide switch

- **Quantity seen:** 1.
- **Appearance:** small metal rectangular body, black sliding handle and three pins.
- **What it does:** the centre pin connects to one outer pin or the other depending on the handle position.
- **GridMind use:** none currently planned.

### Diodes

- **Quantity seen:** several probable diodes; exact count and type not confirmed.
- **Appearance:** small cylindrical bodies with one lead at each end and a stripe near one end.
- **What they do:** allow current to flow mainly in one direction. The stripe marks the cathode.
- **GridMind use:** none currently planned.
- **Identification warning:** do not confuse these with resistors; confirm their markings before use.

### Transistors

- **Quantity seen:** several probable small transistors; exact count and type not confirmed.
- **Appearance:** small black, usually flat-sided bodies with three legs.
- **What they do:** electronically switch or amplify a signal.
- **GridMind use:** none currently planned because the selected LEDs can be driven through suitable resistors without a transistor.
- **Handling note:** pin order varies by transistor type. Do not connect one until its printed part number has been checked.

### Jumper wires

- **Quantity seen:** multiple large bundles, including many male-to-male and male-to-female wires; exact count is unnecessary.
- **Appearance:** short coloured wires with black connector housings or exposed pins at their ends.
- **Types:** male-to-male wires have exposed pins at both ends; male-to-female wires have one pin and one socket; female-to-female wires have sockets at both ends.
- **GridMind use:** male-to-male wires will connect the NodeMCU and components through a solderless breadboard.

### Solderless breadboard

- **Quantity seen:** 4 total: 2 full-size boards and 2 half-size boards.
- **Appearance:** white board containing a grid of connection holes.
- **How it works:** groups of holes are electrically joined underneath, allowing temporary circuits without soldering.
- **GridMind use:** one breadboard per node is preferred for the completed prototype.

### LDR/photoresistor

- **Quantity seen:** 1.
- **Backup stock selected for purchase:** 20 GL5528/5528 5 mm photoresistors, Amazon.fr ASIN `B0897LDR9N`; purchase/delivery not yet confirmed.
- **Appearance:** small round light-sensitive face with a visible winding or zigzag track and two legs.
- **What it does:** its resistance changes with the amount of light falling on it.
- **GridMind use:** Node A solar-availability proxy. Bright and covered readings will be calibrated as game inputs rather than treated as real solar-power measurements.
- **Handling note:** do not connect it to A0 yet. The voltage-divider circuit and the exact safe A0 input arrangement for the owned NodeMCU must be reviewed first.

### RFID reader, card and key fob

- **Quantity seen:** 1 MFRC522-style reader board, 1 white card and 1 blue key fob.
- **Appearance:** blue reader circuit board with a large printed antenna loop; the card and fob contain passive RFID tags.
- **What it does:** reads the identifier stored in a compatible contactless card or fob when it is brought close to the reader.
- **Possible GridMind use:** optional scenario selection or session identification after the core game works.
- **Scope decision:** excluded from GridMind's MVP. The reader uses several SPI/GPIO connections, would compete with the planned buttons, LEDs and OLED, and is not needed for the learning outcomes or coursework constraints.
- **Handling note:** the reader is a 3.3 V device. Do not connect it to 5 V.

### Buzzers

- **Quantity seen:** 2 probable buzzers mounted on the Raspberry Pi extension breadboard.
- **Appearance:** round black sounders; one is marked with a `+`, and the other has a removable protective label reading `REMOVE SEAL AFTER WASHING`.
- **Type:** active versus passive type not yet confirmed.
- **Possible GridMind use:** optional audible warning or decision feedback.
- **Scope decision:** exclude buzzers from the initial MVP unless later pin-map review identifies a clear learning or accessibility benefit.

### 10 kOhm resistors

- **Quantity seen:** one strip containing many resistors; exact count not yet confirmed.
- **Appearance:** small blue cylindrical bodies with coloured bands and one thin metal lead from each end.
- **Identification evidence:** the storage strip is clearly labelled `10KOhm` in `resistors-1.png`.
- **GridMind use:** suitable as the fixed resistor in the planned LDR voltage-divider circuit, after the NodeMCU A0 input arrangement has been confirmed.
- **Handling note:** resistor orientation does not matter; either lead may face either direction.

### 220 ohm resistors

- **Quantity seen:** one strip containing many resistors; exact count not yet confirmed.
- **Appearance:** small blue cylindrical bodies with coloured bands and one thin metal lead from each end.
- **Identification evidence:** the storage strip is clearly labelled `220Ohm` in `resistor-values.jpeg`.
- **GridMind use:** current-limiting resistors for ordinary LEDs. One resistor is required for every LED channel.
- **Handling note:** 220 ohms is suitable for the planned LED tests; 330 ohm resistors are not required when these are available.

### 1 kOhm resistors

- **Quantity seen:** one strip containing many resistors; exact count not yet confirmed.
- **Identification evidence:** the storage strip is clearly labelled `1KOhm` in `resistor-values.jpeg`.
- **GridMind use:** useful spare components, but not currently assigned to the minimum viable circuit.

### Rotary potentiometers

- **Quantity seen:** at least 3.
- **Appearance:** round metal bodies with a rotating shaft and three electrical terminals.
- **What they do:** act as adjustable resistors and can produce a variable voltage for an analogue input.
- **Contingency use:** one may replace the LDR as a learner-controlled renewable-availability dial if the single LDR fails.
- **Handling note:** do not connect a potentiometer to A0 until its resistance, wiring and the NodeMCU A0 input arrangement have been checked with the multimeter.

## MVP component contingencies

- **RGB LED failure:** until the backup pack arrives, use one or more of the confirmed ordinary red, yellow, green or blue LEDs. The RGB LED is not required for the MVP. Even after delivery, prefer an ordinary LED if the three-channel RGB device does not fit the reviewed pin map.
- **LDR failure:** until the GL5528 backup pack arrives, use a rotary potentiometer as an explicitly labelled renewable-availability control. If analogue input remains unavailable, use the Scenario/Next button to select predetermined synthetic renewable values.
- **OLED delay or failure:** retain Serial Monitor, ordinary LEDs and browser dashboards as the working outputs.
- **Dual USB adapter delay:** develop and test one board at a time from the computer. Two-node powered integration waits for a suitable second USB power connection.

## Unidentified items

- Several red, yellow, green and blue square components appear in the first photograph. Their type and pin layout are not sufficiently clear to identify safely.
- Any unclear component should be photographed separately from above and from the side, with its legs and printed markings visible.

## Final checklist and pending deliveries

| Component | Required or useful quantity | Identification cue | Status |
|---|---:|---|---|
| 220 ohm resistors | At least 4 | Blue cylindrical bodies; strip labelled `220Ohm` | Confirmed; exact count pending |
| 330 ohm resistors | Alternative to 220 ohms | Often orange-orange-brown-gold bands | Not present; not required |
| 10 kOhm resistors | At least 1 | Blue cylindrical bodies; strip labelled `10KOhm` | Confirmed; exact count pending |
| LDR/photoresistor | 1 | Round disc with a visible zigzag track and two legs | Confirmed: 1 |
| Buzzer | Optional | Small black cylinder, often marked `+` | Confirmed: 2 probable units; type pending |
| Breadboards | 2 preferred | White boards with rows of holes | Confirmed: 2 full-size and 2 half-size |
| Male-to-male jumper wires | Several | Exposed metal pin at both ends | Confirmed: many |
| Male-to-female jumper wires | Several | Exposed pin at one end and socket at the other | Confirmed: used in LED proof circuit |
| UNI-T UT33D+ multimeter | 1 | Handheld meter with red and black probes | Not yet delivered |
| SSD1309 OLED modules | 2 | Four pins labelled GND, VDD/VCC, SCL and SDA | Not yet delivered |
| Goobay dual USB power adapter | 1 | Dual USB-A mains adapter | Not yet delivered |
| GL5528 photoresistor backup pack | 20 | 5 mm light-dependent resistors marked/sold as GL5528/5528 | Selected for purchase; delivery unconfirmed |
| Common-cathode RGB LED backup pack | 100 | Four-pin, 5 mm diffused RGB LEDs | Selected for purchase; delivery unconfirmed |

## External LED proof setup

- Red LED long leg: `A34`.
- Red LED short leg: `A35`.
- 220 ohm resistor: `B34` to `B30`.
- Green male-to-female jumper: `C30` to NodeMCU `D5`.
- Black male-to-female jumper: `B35` to NodeMCU `G`/ground immediately above `D5`.
- Pin placement was visually checked while unpowered.
- Evidence: `evidence/day2/led-circuit-unpowered.png` and `evidence/day2/led-circuit-pin-check.png`.
- Sketch: `firmware/day2_led_test/day2_led_test.ino`.

The circuit compiled, uploaded and passed physical verification on 7 July 2026. The red LED turned on and off at one-second intervals as intended. Successful upload evidence is `evidence/day2/day2_ext-LED-test.png`; powered-circuit evidence is `evidence/day2/external-led-test-photo-original.heic` with preview `evidence/day2/external-led-test-photo-preview.jpeg`.

## Verified Node B control allocation

- Three pushbuttons are in active use on Board 2: Run=`D5`, Defer=`D6`, Reduce=`D7`.
- All three use Arduino `INPUT_PULLUP` and return to one shared ground rail.
- Jumper convention: Run yellow, Defer orange, Reduce green and ground black.
- The 30 ms non-blocking debounce sketch compiled, uploaded and passed physical press/release testing on 8 July 2026.
- Sketch: `firmware/day2_three_button_test/day2_three_button_test.ino`.
- Evidence: `evidence/day2/node-b-three-buttons-final-unpowered.heic`, `evidence/day2/node-b-three-buttons-final-unpowered.png` and `evidence/day2/day2_three_button_test.png`.

## Planned Node B output allocation

- One ordinary feedback LED is assigned to `D0`/GPIO16 with one 220 ohm series resistor; this circuit is not yet connected or verified.
- `D1`/GPIO5 and `D2`/GPIO4 remain reserved for the future I2C OLED.
- `D3`/GPIO0, `D4`/GPIO2 and `D8`/GPIO15 are excluded from added Node B devices because they affect ESP8266 boot mode.
- The RGB LED remains unused because it would require three GPIO channels and one resistor per channel.

## External-component-limit note

The backup LDR and RGB LED packs duplicate generic component types already present in the prerequisite kit and are contingency stock, not added functionality. Do not assume they are automatically exempt from the coursework limit. Prefer the original kit components and obtain lecturer clarification before relying on both externally purchased backup types in the submitted build. The two ordered OLED modules currently consume two of the three allowances under the strict interpretation.
