package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strings"

	metaf "github.com/akhenakh/gometaf"
)

func main() {
	reportInput := flag.String("report", "", "METAR/TAF report to parse")
	outputFormat := flag.String("format", "text", "Output format: text or json")
	prettyJson := flag.Bool("pretty", false, "Pretty-print JSON output (only applicable with json format)")
	flag.Parse()

	if *reportInput == "" {
		fmt.Fprintln(os.Stderr, "Error: Please provide a METAR/TAF report using the -report flag.")
		flag.Usage()
		os.Exit(1)
	}

	// Validate output format
	format := strings.ToLower(*outputFormat)
	if format != "text" && format != "json" {
		fmt.Fprintf(os.Stderr, "Error: Invalid format '%s'. Must be 'text' or 'json'.\n", *outputFormat)
		os.Exit(1)
	}

	explanation, err := metaf.ExplainReport(*reportInput)
	if err != nil {
		// Handle errors returned by the library (e.g., empty input)
		fmt.Fprintf(os.Stderr, "Error explaining report: %v\n", err)
		os.Exit(1)
	}

	if format == "json" {
		// Output as JSON
		var jsonData []byte
		if *prettyJson {
			jsonData, err = json.MarshalIndent(explanation, "", "  ")
		} else {
			jsonData, err = json.Marshal(explanation)
		}

		if err != nil {
			fmt.Fprintf(os.Stderr, "Error marshaling JSON: %v\n", err)
			os.Exit(1)
		}

		fmt.Println(string(jsonData))
	} else {
		// Output as plain text (default)
		fmt.Printf("Report Type: %s\n", explanation.ReportType)
		if explanation.Error != "" {
			fmt.Printf("Parsing Issue: %s\n", explanation.Error)
		}

		// Print other metadata flags if they are set or relevant
		if explanation.Location != "" {
			fmt.Printf("Location:    %s\n", explanation.Location)
		}
		if explanation.Timestamp != "" {
			fmt.Printf("Timestamp:   %s\n", explanation.Timestamp)
		}
		if explanation.IsSpeci {
			fmt.Println("Is SPECI:    true")
		}
		if explanation.IsAutomated {
			fmt.Println("Is Automated:true")
		}
		if explanation.IsNil {
			fmt.Println("Is NIL:      true")
		}
		if explanation.IsCancelled {
			fmt.Println("Is Cancelled:true")
		}
		if explanation.IsAmended {
			fmt.Println("Is Amended:  true")
		}
		if explanation.IsCorrectional {
			fmt.Println("Is Correctnl:true")
		}

		fmt.Println("\n--- Groups ---")

		// Find the maximum length of raw groups for alignment
		maxGroupLen := 0
		for _, group := range explanation.Groups {
			if len(group.RawGroup) > maxGroupLen {
				maxGroupLen = len(group.RawGroup)
			}
		}
		// Ensure minimum width, but allow for longer groups
		if maxGroupLen < 15 {
			maxGroupLen = 15
		}
		alignFormat := fmt.Sprintf("%%-%ds  ", maxGroupLen) // e.g., "%-15s  "

		for _, group := range explanation.Groups {
			// Handle multi-line explanations for aligned plain text output
			lines := strings.Split(group.Explanation, "\n")
			fmt.Printf(alignFormat, group.RawGroup) // Print the raw group aligned
			fmt.Printf("%s\n", lines[0])            // Print the first line of explanation

			// Print subsequent lines indented
			if len(lines) > 1 {
				indent := strings.Repeat(" ", maxGroupLen+2) // Spaces for alignment + 2 spaces separator
				for _, line := range lines[1:] {
					fmt.Printf("%s%s\n", indent, line)
				}
			}
		}
	}
}
