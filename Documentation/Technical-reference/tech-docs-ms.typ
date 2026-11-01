
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
  ==== #raw(name)
  #raw(sig, lang: "cpp", block: true)
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
    == #raw(name + ".hpp") / #raw(name + ".cpp")
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
    [R1], [RGB Color sensors will capture ambient light data], [Required],
    [R1.1], [System will use the I2C protocol to communicate with the TCS34725 RGB Color sensor], [Planned],
    [R1.2], [System must react using RGB color sensors to colored tiles in real time], [Planned],
    [R1.3], [System will be capable of toggling the built-in LED illumination of the color sensors on/off in software], [Planned],
    [R2], [System will output tones over DAC to 2 small speakers], [Planned],
    [R2.1], [System will map 8 tones to 8 individual colors], [Planned],
    [R2.2], [System will be capable of playing a note for each of the 3 sensor tracks], [Required],
    [R2.2.1], [System will be capable of playing multi-note chords], [Planned],
    [R2.3], [System will be able to 'glyssando' linearly between each of the 8 planned tones], [Want],
    [R3], [System will provide debug output over serial UART to a PC during normal operation], [Required],
    [R3.1], [Debug output will include the initialization feedback], [Planned],
    [R3.2], [Debug output will include the readings from each of the RGB color sensors], [Planned],
    [R3.3], [Debug output will include the output note / DAC information], [Planned],
)

== System Summary
This section is a short description of how the pieces fit together. The System block diagram
is shown in @fig:block-diagram and the schematic detailed in @sec:hardware. The main microcontroller is an Arduino Giga R1 Wifi driving the following; 2 5W speakers via DAC and a Class-D amplifier breakout for the audio output, and 3 TCS34725 RGB Color sensors via individual I2C busses. The Power supply supplies the Giga and then all other devices are powered on the 3.3V out pins provided by it. The reason for the overkill MCU is pinout real-estate. Each of the RGB color sensors have the same exact I2C address, making a single bus impossible. DAC cannot drive the speakers alone, hence the inclusion of the amplifier with a true system reference ground for interference mitigation.

== Revision History
#table(
  columns: (auto, auto, 1fr),
  table.header[Version][Date][Changes],
    [0.1], [October 31st, 2026], [Initial draft],
)

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
    [`rgbTCS34725.hpp`], [Structure definitions, constants and function prototypes for .cpp counterpart],
  [`rgbTCS34725.cpp`], [Interface functions for I2C driven TCS34725 RGB color sensor],
  [`main.cpp`], [Entry point: `setup()` and `loop()`],
)

#pagebreak() // Appears to do nothing but don't be fooled....

#file-pair(
  "rgbTCS34725",
    [This pair defines and implements the RGB color sensing.],
  hpp: [Declares the `Color` data structure etc, function prototypes, and constants for RGB sensors],
    cpp: [Implements the 'driver' for these sensors. Including Read, Write, and Batched-Read color sampling from registers.],
  uses: [`Wire.h`, `stdint.h`],
)

=== Functions <sec:func-rgb>

The following functions exist as a simple hardware API for interfacing with the I2C driven configuration, control, and data registers on the TCS34725 RGB color sensor.

#fn-entry(
    "initSensor()",
    "bool initSensor(TwoWire &bus);",
    [Initializes the I2C bus and RGB sensor it is called upon. Call once for each connected bus-sensor pair from `setup()`. The enumeration in the paired .hpp file must be updated, but `NUM_SENSORS` can be used as the limit for a for loop iteration over this function to get the job done.],
  file: "rgbTCS34725.cpp",
    returns: [A `bool` whose value is `True` if init success; `False` otherwise. Note: expects the TCS34725 specifically and will return false for any other correctly connected sensor. See definition.],
)

#fn-entry(
    "writeReg()",
    "void writeReg(TwoWire &bus, uint8_t reg, uint8_t val);",
    [Writes to a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single write operation (of `val` to `reg`) using the effective message (`CMD` | `reg`).],
  file: "rgbTCS34725.cpp",
    returns: [Nothing.],
)

#fn-entry(
    "readReg()",
    "uint8_t readReg(TwoWire &bus, uint8_t reg);",
    [Reads a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single read operation using the effective message (`CMD` | `reg`) and subsequent bus read.],
  file: "rgbTCS34725.cpp",
    returns: [A `uint8_t` whose value represents the byte contents of a `reg`],
)

#pagebreak()

#fn-entry(
    "readColor()",
    "Color readColor(TwoWire &bus);",
    [Writes to a given register address given an I2C bus to preform the operation on. ADDR is the same for all sensors and unessecary to pass. Commands a static register address for a single write operation using the effective message (`CMD` | `reg`).],
  file: "rgbTCS34725.cpp",
    returns: [A `Color` struct containing four usable `uint16_t` halfwords for each of the combined Low and High bytes for each of the colors' registers. See rgbTCS34725.hpp],
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
TCS3472X datasheet (TAOS135, August 2012), https://cdn-shop.adafruit.com/datasheets/TCS34725.pdf
