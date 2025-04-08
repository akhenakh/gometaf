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
