package main

import (
	"flag"
	"fmt"
	"html"
	"os"
	"strings"

	metaf "github.com/akhenakh/gometaf" // Import the SWIG generated wrapper
)

// ExplainResult represents a result from explaining a METAR/TAF group
type ExplainResult struct {
	RawGroup    string
	Explanation string
}

// MetafExplainer explains METAR/TAF reports
type MetafExplainer struct{}

// Explain parses and explains a METAR/TAF report
func (m *MetafExplainer) Explain(input string) ([]ExplainResult, string, string) {
	// Parse the METAR/TAF report
	parseResult := metaf.ParseMetaf(input)

	// Get the original groups from the input
	// Split by spaces but keep groups that might contain spaces inside quotes together
	originalGroups := splitMaintainingQuotedGroups(input)

	// Prepare our results
	var results []ExplainResult

	// Get report type text
	reportType := parseResult.GetReportType()

	// Get error text if any
	errorText := ""
	if parseResult.GetError() != "" {
		errorText = fmt.Sprintf("Parsing error: %s", parseResult.GetError())
	}

	// Get explanations
	explanations := parseResult.GetRawGroups()

	// Process each group
	for i := 0; i < int(explanations.Size()); i++ {
		explanation := explanations.Get(i)

		// Find the raw group from original input
		rawGroup := ""
		if i < len(originalGroups) {
			rawGroup = originalGroups[i]
		}

		// Replace newlines with HTML breaks for HTML output
		explanation = strings.ReplaceAll(explanation, "\n", "<br>")

		results = append(results, ExplainResult{
			RawGroup:    rawGroup,
			Explanation: explanation,
		})
	}

	return results, reportType, errorText
}

// Split a string by spaces while keeping quoted sections together
func splitMaintainingQuotedGroups(s string) []string {
	var result []string
	var current string
	inQuotes := false

	// Replace the report end character with space to avoid it being part of a group
	s = strings.Replace(s, "=", " ", -1)

	for _, char := range s {
		if char == '"' {
			inQuotes = !inQuotes
			continue
		}

		if char == ' ' && !inQuotes {
			if current != "" {
				result = append(result, current)
				current = ""
			}
		} else {
			current += string(char)
		}
	}

	if current != "" {
		result = append(result, current)
	}

	return result
}

func main() {
	// Parse command line flags
	report := flag.String("report", "", "METAR/TAF report to parse")
	outputHTML := flag.Bool("html", false, "Output as HTML table")
	flag.Parse()

	// Check if a report was provided
	if *report == "" {
		fmt.Println("Please provide a METAR/TAF report using the -report flag")
		flag.Usage()
		os.Exit(1)
	}

	// Create the explainer
	explainer := &MetafExplainer{}

	// Explain the report
	results, reportType, errorText := explainer.Explain(*report)

	// Output the results
	if *outputHTML {
		fmt.Println("<table border='1'>")
		fmt.Println("<thead><tr><th>Group</th><th>Explanation</th></tr></thead>")
		fmt.Println("<tbody>")

		fmt.Printf("<tr><td>&nbsp;</td><td>Detected report type: %s</td></tr>\n",
			escapeHTML(reportType))

		if errorText != "" {
			fmt.Printf("<tr><td>&nbsp;</td><td>%s</td></tr>\n", escapeHTML(errorText))
		}

		for _, result := range results {
			fmt.Printf("<tr><td>%s</td><td>%s</td></tr>\n",
				escapeHTML(result.RawGroup), result.Explanation)
		}

		fmt.Println("</tbody></table>")
	} else {
		fmt.Printf("Report type: %s\n\n", reportType)

		if errorText != "" {
			fmt.Printf("%s\n\n", errorText)
		}

		for _, result := range results {
			// Ensure a minimum width for alignment
			fmt.Printf("%-20s %s\n", result.RawGroup,
				strings.ReplaceAll(result.Explanation, "<br>", "\n                     "))
		}
	}
}

// HTML escape function
func escapeHTML(s string) string {
	return html.EscapeString(s)
}
