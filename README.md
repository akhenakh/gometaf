# GoMetaf

GoMetaf is a Go wrapper for the [metaf](https://github.com/nnaumenko/metaf) C++ library, providing aviation weather report parsing capabilities for METAR and TAF formats in Go applications.

## Overview

This library allows you to parse and explain METAR (Meteorological Aerodrome Report) and TAF (Terminal Aerodrome Forecast) reports in your Go applications. It uses SWIG to provide a Go interface to the C++ metaf library.

The project includes a command-line tool `explain` that demonstrates how to use the library to parse and explain METAR/TAF reports.

## Features

- Parse METAR and TAF reports
- Get detailed explanations of each group in the report
- Output explanations in plain text or HTML format
- Handle standard aviation weather report formats

## Installation

### Prerequisites

- Go
- SWIG 4.0 or higher
- C++ compiler (gcc, clang)
- The original metaf C++ library (included for convenience)

### Building

The included Makefile handles the building process:

```bash
# Clone the repository
git clone https://github.com/akhenakh/gometaf.git
cd gometaf

# Build the project
make
```

This will:
1. Compile the C++ metaf library
2. Generate the SWIG wrapper
3. Compile the Go wrapper package
4. Build the `explain` & `metaf` example command-line tool

## Usage

### Command-line Tool

The `explain` tool demonstrates how to use the library:

```bash
# Explain a METAR report
explain -report "METAR KSFO 081656Z 28011KT 10SM FEW013 BKN042 16/09 A3007 RMK AO2 SLP182 T01610094"
````
## As As Code

```Go
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"

	metaf "github.com/akhenakh/gometaf"
)

func main() {
	reportStr := flag.String("report", "METAR KLAX 091953Z 25005KT 10SM FEW040 SCT060 BKN120 19/13 A2994 RMK AO2 SLP138 T01890133=", "METAR/TAF report string")
	pretty := flag.Bool("pretty", false, "Pretty-print JSON output")
	flag.Parse()

	if *reportStr == "" {
		fmt.Fprintln(os.Stderr, "Error: -report flag cannot be empty")
		flag.Usage()
		os.Exit(1)
	}

	explanation, err := metaf.ExplainReport(*reportStr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error explaining report: %v\n", err)
		os.Exit(1)
	}

	var jsonData []byte
	if *pretty {
		jsonData, err = json.MarshalIndent(explanation, "", "  ")
	} else {
		jsonData, err = json.Marshal(explanation)
	}

	if err != nil {
		fmt.Fprintf(os.Stderr, "Error marshaling JSON: %v\n", err)
		os.Exit(1)
	}

	fmt.Println(string(jsonData))
}
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- This project is based on the [metaf](https://github.com/nnaumenko/metaf) C++ library by Nikolai Naumenko
- The original metaf library is included for convenience
