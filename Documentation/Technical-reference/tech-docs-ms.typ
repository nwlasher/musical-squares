
// =====================================================================
// Technical document template
// Fed claude pieces of typst documentation and had it whip up and interactive
// template for me to play with to learn.
//   Compile:  typst compile tech-doc-template.typ
//   Live:     typst watch tech-doc-template.typ
// Search for TODO to find the parts you need to fill in.
// =====================================================================

// ---------- 1. Metadata: edit these ----------------------------------
#let doc-title    = "Project 3: Musical Squares"
#let doc-subtitle = "Technical Reference Document"
#let doc-author   = "Nathaniel Lasher"
#let doc-course      = "EE526 - Wearable and Implantable Sensors"
#let doc-org =    "The University at Buffalo"
#let doc-version  = "0.1"
#let doc-date     = datetime.today().display("[month repr:long] [day], [year]")

// ---------- 2. Document-wide settings --------------------------------
#set document(title: doc-title, author: doc-author)
#set text(size: 11pt)
#set par(justify: true)
#set heading(numbering: "1.1")
#show raw: set text(size: 0.9em)
#set table(
  stroke: 0.5pt + luma(180),
  inset: 6pt,
  fill: (_, y) => if y == 0 { luma(235) },   // shade the header row
)
#show figure.caption: set text(size: 0.9em)

// ---------- 3. Helpers (used in the sections below) ------------------

// Dashed box standing in for a diagram until you have the real image.
#let placeholder(label, h: 7cm) = rect(
  width: 100%,
  height: h,
  stroke: (dash: "dashed", paint: gray),
  fill: luma(248),
  align(center + horizon, text(fill: gray)[#label]),
)

// One entry in the function reference.
//   params: array of (name, description) pairs, e.g. (("pin", [GPIO number]),)
#let fn-entry(name, sig, desc, file: none, params: (), returns: none) = [
    #heading(level: 4, raw(name)) #label("fn:" + name.replace("()", ""))
    #raw(sig, lang: "cpp", block: true) // For the highlighted signature
  #desc

  #if file != none [*Defined in:* #raw(file)]

  #if params.len() > 0 [
    #table(
      columns: (auto, 1fr),
      table.header[Parameter][Description],
      ..params.map(p => (raw(p.at(0)), p.at(1))).flatten(),
    )
  ]

  #if returns != none [*Returns:* #returns]
]

// One .cpp/.hpp pair.  name is the shared base name, e.g. "sensor".
#let file-pair(name, purpose, hpp: [], cpp: [], uses: none) = {
  let extra = if uses != none { ([Depends on], uses) } else { () }
  [
    #heading(level: 2)[#raw(name + ".hpp") / #raw(name + ".cpp")] #label("sec:" + name + "-files")
    #purpose

    #table(
      columns: (auto, 1fr),
      table.header[File][Contents],
      raw(name + ".hpp"), hpp,
      raw(name + ".cpp"), cpp,
      ..extra,
    )
  ]
}

// =====================================================================
// TITLE PAGE (no page number)
// =====================================================================
#page(numbering: none, margin: 1.25in)[
  #align(center + horizon)[
    #text(size: 28pt, weight: "bold")[#doc-title]
    #v(0.4em)
    #text(size: 16pt, fill: luma(90))[#doc-subtitle]
    #v(3em)
    #line(length: 40%, stroke: 0.5pt + gray)
    #v(1.5em)
    #doc-author \
      #doc-course / 
      #doc-org
    #v(1em)
    Version #doc-version · #doc-date
  ]
]

// =====================================================================
// FRONT MATTER (roman numerals)
// =====================================================================
#set page(
  paper: "us-letter",
  margin: (x: 1in, y: 1in),
  numbering: "i",
  header: align(right, text(size: 9pt, fill: gray)[#doc-title]),
)
#counter(page).update(1)

#outline(title: [Table of Contents], depth: 4)
#pagebreak()
#outline(title: [List of Figures], target: figure.where(kind: image))
#outline(title: [List of Listings], target: figure.where(kind: raw))

// Optional: uncomment for a list of tables as well.
// #pagebreak()
// #outline(title: [List of Tables], target: figure.where(kind: table))

// =====================================================================
// BODY (arabic numbers, each top-level section starts a new page)
// =====================================================================
#set page(numbering: "1")
#counter(page).update(1)
#show heading.where(level: 1): it => {
  pagebreak(weak: true)
  it
}

= Project Overview

== Purpose
This project, realized for #doc-course, is a project that I have had the idea for on my mind for a while. The purpose was to build your own unique smart toy device(s) using Arduino. I chose to try and work with sensors and protocols I haven't used before to add another layer to it. I was also facinated by the inner workings and split between photodiodes and LEDs. 

== Scope
The scope of this project includes the following aspects explicitly. This was more of a implementation checklist for organizing my thoughts at the start.

== Requirements
#table(
  columns: (auto, 1fr, auto),
    table.header[ID][Requirement][Status],
    [R1], [#strike[RGB Color sensors will capture ambient light data]], [Done],
    [R1.1], [#strike[System will use the I2C protocol to communicate with the TCS3472x RGB Color sensor]], [Done],
    [R1.2], [System must react using RGB color sensors to colored tiles in real time], [Planned],
    [R1.3], [#strike[System will be capable of toggling the built-in LED illumination of the color sensors on/off in software]], [Done],
    [R2], [#strike[System will output tones over DAC to 2 small speakers]], [Done],
    [R2.1], [System will map 8 tones to 8 individual colors], [Planned],
    [R2.2], [System will be capable of playing a note for each of the 3 sensor tracks], [Required],
    [R2.2.1], [System will be capable of playing multi-note chords], [Planned],
    [R2.3], [#strike[System will be able to 'glyssando' linearly between each of the 8 planned tones]], [Done],
    [R3], [#strike[System will provide debug output over serial UART to a PC during normal operation]], [Done],
    [R3.1],[#strike[Debug output will include the initialization feedback]], [Done],
    [R3.2],[#strike[Debug output will include the readings from each of the RGB color sensors]], [Done],
    [R3.3],[#strike[Debug output will include the output note / DAC information]], [Done],
)

== System Summary
This section is a short description of how the pieces fit together. The System block diagram
is shown in @fig:block-diagram and the schematic detailed in @sec:hardware. The main microcontroller is an Arduino Giga R1 Wifi driving the following; 2 5W speakers via DAC and a Class-D amplifier breakout for the audio output, and 3 TCS3472x RGB Color sensors via individual I2C busses. The Power supply supplies the Giga and then all other devices are powered on the 3.3V out pins provided by it. The reason for the overkill MCU is pinout real-estate. Each of the RGB color sensors have the same exact I2C address, making a single bus impossible. DAC cannot drive the speakers alone, hence the inclusion of the amplifier with a true system reference ground for interference mitigation.

== Revision History
#table(
  columns: (auto, auto, 1fr),
  table.header[Version][Date][Changes],
    [#doc-version], [#doc-date], [Initial draft],
)

== Testing
Throughout development I saved each inital stage of working behavior per component to create a directory of harware/software testbenches located in `Testbenches/`. See @sec:repo-layout for more information on the layout of this repository. For more information of the available debug terminal See @sec:debug. The purpose of each test is as follows:

#table(
    columns: (auto, 1fr),
    table.header[Test Name (folder)][Description],
    [`giga-blink/`],[Ol' Relaiable. For getting the environment setup and flashing for the first time.],
    [`rgb-sensor-debug`], [Debug terminal available from this test forward. Configurable to test from a single RGB color sensor up to the hardware limit. Scan the I2C bus and log available devices as well as stream data from the found sensors. Meant to test sensor hardware and interface software.],
    [`audio-debug`], [Adds in the `synth` interface and allows for terminal commanded use to test the software module and speaker/DAC/Amp hardware.],
    [`...`], [More tests will be added as they are made.]
)

=== Debug Interface <sec:debug>

A simple debug terminal is exposed over a configurable port to provide a simple way to poke various system state. Echoing is implemented, though depending on the config it may be unreadable at times. Available commands are as follows:

#table(
    columns: (auto, 1fr),
    table.header[Command][Description],
    [`led`], [Toggles the LEDs onboard each connected TCS3472x Sensor by pulling the `LED` pin low. Feedback is given on success with `led toggled`.],
    [`stream`], [Essentially the same as `led` but for the raw data stream from all connected RGB color sensors. Will toggle on command with successful operation denoted by `sensor debug stream <on/off>`.],
    [`status`], [Will print a table containing system state variables. May make this a live output in the future.],
    [`play [0u-7u] [MIN_FREQf-MAX_FREQf]`], [Exposes `synth` behavior. Tones 0-7 are avalable as well as frequencies from min to max (format 0 and 233.5 respectively). Successful operation echos the selected tone or freq. Track will glyssando on subsequent commands without a `stop`. All command act on track 0. Default argument is ],
    [`stop`],[Silences track 0]
)

Serial debug output can be routed according to the following code screenshot. Note that all raw sensor data is routed to `DEBUG_SERIAL`, while all terminal is routed through `DTERM_SERIAL`. See `main.ccp` for the code shown in @fig:debug-out-and-config. Note also for any `Serial1` output that a USB-UART bridge is needed to capture Tx/Rx data and relay to a PC or similar terminal.

#figure(
  grid(
    columns: (2fr, 1fr),
    column-gutter: 1em,
    align: horizon,
    image("Images/debug-out-config.jpeg", width: 100%),
    image("Images/debug-out.jpg", width: 100%),
  ),
  kind: image,
  caption: [Left: Define macro for debug output routing. Right: Debug output example.],
) <fig:debug-out-and-config>

// ---------------------------------------------------------------------
= Repository Layout <sec:repo-layout>

This section provides a brief description of repository layout, source file roles and each function included in the project, each subsection corresponds to a separate source file.
Each module is a header and implementation pair; `main.cpp` has no header.
Below is a tree diagram of the repo layout and a table describing source roles.


// Formats the repo layout output from the 'tree' command into a figure; Run ./map-repo.sh from top level to update
#let repo-tree(path: "repo-tree.txt", caption: [Repository layout.]) = figure(
  block(width: 100%, inset: 8pt, stroke: 0.5pt + luma(180), fill: luma(248), breakable: false)[
    #set align(left)
    #set text(size: 0.85em)
    #raw(read(path), block: true)
  ],
  kind: raw,
  supplement: [Listing],
  caption: caption,
)

// Call to place into document
#repo-tree() <fig:repo-tree>

// General table for source file details
#table(
  columns: (auto, 1fr),
  table.header[Module][Responsibility],
    [`TCS3472x.hpp`], [Structure definitions, constants and function prototypes for .cpp counterpart],
  [`TCS3472x.cpp`], [Interface functions for I2C driven TCS3472x RGB color sensor],
  [`main.cpp`], [Entry point: `setup()` and `loop()`],
)

#pagebreak() // Appears to do nothing but don't be fooled....

#file-pair(
  "TCS3472x",
    [This pair defines and implements the RGB color sensing. `N` Sensors are supported via a single enumeration `Sensor_Id` that (save for the debug output and I2C mappings) is completely configurable by adjusting that single point; For example running `rgb-sensor-debug/` with one sensor is possible by commenting out lines in the corresponding header file and functionality will follow suit. Sensors report ambient light and color data in real time via the data structures and interface funtions defined within this section],
  hpp: [Declares the `Color` data structure etc, function prototypes, and constants for RGB sensors],
    cpp: [Implements the 'driver' for these sensors. Including Read, Write, and Batched-Read color sampling from registers.],
  uses: [`Wire.h`, `stdint.h`],
)

=== Functions <sec:func-rgb>

The following functions exist as a simple hardware API for interfacing with the I2C driven configuration, control, and data registers on the TCS3472x RGB color sensor.

#fn-entry(
    "initSensor()",
    "bool initSensor(TwoWire &bus);",
    [Initializes the I2C bus and RGB sensor it is called upon. Call once for each connected bus-sensor pair from `setup()`. The enumeration in the paired .hpp file must be updated, but `NUM_SENSORS` can be used as the limit for a for loop iteration over this function to get the job done.],
  file: "TCS3472x.cpp",
    returns: [A `bool` whose value is `True` if init success; `False` otherwise. Note: expects the TCS3472x specifically and will return false for any other correctly connected sensor. See definition.],
)

#fn-entry(
    "writeReg()",
    "void writeReg(TwoWire &bus, uint8_t reg, uint8_t val);",
    [Writes to a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single write operation (of `val` to `reg`) using the effective message (`CMD` | `reg`).],
  file: "TCS3472x.cpp",
    returns: [Nothing.],
)

#fn-entry(
    "readReg()",
    "uint8_t readReg(TwoWire &bus, uint8_t reg);",
    [Reads a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single read operation using the effective message (`CMD` | `reg`) and subsequent bus read.],
  file: "TCS3472x.cpp",
    returns: [A `uint8_t` whose value represents the byte contents of a `reg`],
)

#pagebreak()

#fn-entry(
    "readColor()",
    "Color readColor(TwoWire &bus);",
    [Writes to a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single write operation using the effective message (`CMD` | `reg`).],
  file: "TCS3472x.cpp",
    returns: [A `Color` struct containing four usable `uint16_t` halfwords for each of the combined Low and High bytes for each of the colors' registers. See TCS3472x.hpp],
)

#file-pair(
    "synth",
    [These files provide the DAC interface functions required for tone generation. The pair provides methods to play, stop, blend, and ramp or 'glide' as referred to later tone on 1-2 speakers. Volume is adjustable in software via the `AMPLITUDE` constant. Dynamic range of the speakers is defined by `MAX_FREQ` and `MIN_FREQ`.],
    hpp: [Defines the Tone enum and Track structure as well as constants and function interfaecs. Configurability follows that of @sec:TCS3472x-files],
    cpp: [Source implementation for the above.],
    uses: [`math.h`, `Arduino_AdvancedAnalog.h`, `Arduino.h`]
    
)

=== Functions <sec:synth-func>

#fn-entry(
    "synthInit()",
    "void synthInit();",
    [Initializes the DAC and DMA hardware required for tone generation. Responsible for the pre-calculation of the Sine approximation table `SINE_TABLE` and configuration of `SAMPLE_RATE`, `BUF_SAMPLES`, `BUF_COUNT`, and the analog resolution used during operation.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "synthUpdate()",
    "void synthUpdate();",
    [Populates the DMA buffer responsible for feeding the DAC in real time. MUST be called imediately in every execution of loop; Loop should have no calls to `delay()`, See @sec:main.cpp. When a DAC/DMA buffer becomes free, this function will populate it. Prior to population, it calls `advanceGlides()` to update each track's `phase`. See @fn:advanceGlides.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "advanceGlides()",
    "void advanceGlides();",
    [On call, moves every track that is currently changing tone or 'gliding' one `step` closer to it's `target_hz` by incrementing `hz+=hz_per_buffer`. When a track has reached it's target frequency (`hz == target_hz`), this function returns with no effect. Rate of change of `hz` is defined by the glide provoking call from `GlideTo` functions. See @fn:synthGlideTo and @fn:synthGlideToHz.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "stepFor()",
    "float stepFor(float hz);",
    [Simply calculates the `step` field value. This value represents the distance of each 'step' through the `SINE_TABLE`. This value is used in `synthUpdate()` to increment `phase`. See @fn:synthUpdate.],
    file: "synth.cpp",
    returns: [`float` representing the change in `phase` index (into `SINE_TABLE`) to be applied at next update]
)

#fn-entry(
    "clampFreq()",
    "float clampFreq(float hz);",
    [Clamps the value of `hz` to the dynamic range of the installed speakers as set by the `MIN_FREQ` and `MAX_FREQ` constants.],
    file: "synth.cpp",
    returns: [Returns `MIN_FREQ` if `hz < MIN_FREQ`, `MAX_FREQ` if `hz > MAX_FREQ`, and `hz` otherwise.]
)

#fn-entry(
    "synthGlideTo()",
    "void synthGlideTo(uint8_t track, Tone tone, uint32_t ms);",
    [Converts a `Tone` into hz via the enumeration value passed into `tone`, then calls `synthGlideToHz()` on this value, applying it to `track` smoothly over a period of `ms` milliseconds. See @fn:synthGlideToHz.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "synthGlideToHz()",
    "void synthGlideToHz(uint8_t track, float hz, uint32_t ms);",
    [When called on an active `track` with a `hz` value different to `track.hz`, configures the track to shift it's tone to the new target frequency over a period of `ms` milliseconds. Responsible for calculating the `hz_per_buffer` field. Calls `synthPlayHz()` on an inactive or non-gliding track. Can be called with an arbitrary frequency within the dynamic range defined by `MIN_FREQ` and `MAX_FREQ`. See @fn:clampFreq and @fn:synthPlayHz.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "synthPlay()",
    "void synthPlay(uint8_t track, Tone tone);",
    [Converts a `Tone` into hz via the enumeration value passed into `tone`, then calls `synthPlayHz()` on this value, applying it to `track`. See @fn:synthPlayHz.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "synthPlayHz()",
    "void synthPlayHz(uint8_t track, float hz);",
    [When called on an active `track` with a `hz` value different to `track.hz`, configures the track to imediately change it's tone to the new frequency. Will activate non-active tracks automatically. Can be called with an arbitrary frequency within the dynamic range defined by `MIN_FREQ` and `MAX_FREQ`. See @fn:clampFreq.],
    file: "synth.cpp",
    returns: [Nothing.]
)

#fn-entry(
    "synthStop()",
    "void synthStop(uint8_t track);",
    [Imediately stops a tracks playback by setting `active` to `false`.],
    file: "synth.cpp",
    returns: [Nothing.],
)

== main.cpp <sec:main.cpp>

// ---------------------------------------------------------------------

// Tip: to show real source instead of a description, read it from disk.
// The path is relative to this .typ file:
//
//   #raw(read("src/sensors.hpp"), lang: "cpp", block: true)

// ---------------------------------------------------------------------
= Hardware <sec:hardware>

== Components
#table(
  columns: 5,
  table.header[Part][Description][Qty][Interface][Cost],
    [Arduino Giga R1 Wifi], [STM32 based microcontroller with 3 I2C lanes and DAC.], [1], [USB, JTAG, UART], [\$72.82],
    [TCS-34725], [RGB Color/Light Sensor], [1], [I2C], [\$18.02],
    [PAM8403], [Dual Channel Class-D Amplifier Breakout Board], [1], [DAC],[\$5.99],
    [Speaker], [Generic Amazon 5W 8 Ohm speaker], [2], [Analog], [\$7.85],
)

== Pin Assignments
#table(
  columns: (auto, auto, 1fr),
  table.header[Pin][Signal][Connected to],
  [`D2`], [`SDA`], [TODO],
  [`D3`], [`SCL`], [TODO],
)

== Power
TODO: Supply voltage, regulators, and estimated current draw. Currently wall power.

// ---------------------------------------------------------------------
= Block and Schematic Diagrams <sec:diagrams>

== Block Diagram
Initial hardware planning block diagram.

#figure(
    image("Images/IMG_2811.jpeg", width: 80%),
  kind: image,
  caption: [System block diagram.],
) <fig:block-diagram>
#pagebreak()

== Schematic
Hardware schematic showing the pin connections, circuit components and power routing. 
#figure(
  placeholder([Schematic goes here], h: 9cm),
  kind: image,
  caption: [Circuit schematic.],
) <fig:schematic>

// To use real images, export them as SVG or PNG and replace the
// placeholder(...) call with:
//
//   image("diagrams/block.svg", width: 100%)
//
// Keep `kind: image` on the figure so it appears in the List of Figures.

= References

TODO: This section should include a full list of references. Note that I setup typst, emacs, and platformio for the first time with this project so I'm including that material.

 --- Typst (official) ---
https://typst.app/docs/
https://typst.app/docs/reference/
https://typst.app/docs/tutorial/
https://typst.app/docs/reference/syntax/
https://typst.app/docs/reference/scripting/
https://typst.app/docs/reference/styling/
https://typst.app/docs/reference/model/figure/
https://typst.app/docs/reference/model/outline/
https://typst.app/docs/reference/model/table/
https://typst.app/docs/reference/text/raw/
https://typst.app/docs/reference/layout/page/
https://typst.app/docs/reference/data-loading/read/
https://typst.app/universe/
https://github.com/typst/typst/releases

--- Typst (community / editor tooling) ---
https://sitandr.github.io/typst-examples-book/
https://github.com/Ziqi-Yang/typst-ts-mode
https://zff.dev/meow_king/typst-ts-mode/wiki/Installation
https://github.com/Myriad-Dreamin/tinymist
https://github.com/uben0/tree-sitter-typst

--- PlatformIO ---
https://docs.platformio.org/
https://docs.platformio.org/en/stable/integration/ide/emacs.html
https://github.com/zachmassia/platformio-mode
https://bit.ly/package-manager-ioerror
https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules
https://community.platformio.org/t/arduino-giga-r1-wifi-board-configuration/34409
https://community.platformio.org/t/giga-dfu-issues/41982

--- Emacs ---
https://www.gnu.org/software/emacs/manual/
https://www.gnu.org/software/emacs/manual/html_node/use-package/
https://www.gnu.org/software/emacs/manual/html_mono/eglot.html
https://www.gnu.org/software/emacs/manual/html_node/elisp/index.html
https://www.gnu.org/software/auctex/manual/auctex/index.html
https://melpa.org/

--- Language servers and language modes ---
https://clangd.llvm.org/
https://pkg.go.dev/golang.org/x/tools/gopls
https://github.com/dominikh/go-mode.el
https://github.com/python-lsp/python-lsp-server
https://github.com/chipsalliance/verible
https://github.com/latex-lsp/texlab

--- RGB Color Sensor documentation
TCS3472X datasheet (TAOS135, August 2012), https://cdn-shop.adafruit.com/datasheets/TCS3472x.pdf
