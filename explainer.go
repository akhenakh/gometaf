package metaf

import (
	"fmt"
	"strings"
)

// GroupExplanation holds the raw group string and its explanation.
// JSON tags are added for clear serialization.
type GroupExplanation struct {
	RawGroup    string `json:"raw_group"`
	Explanation string `json:"explanation"`
}

// ReportExplanation holds the full parsed and explained METAR/TAF report.
// JSON tags are added for clear serialization, using omitempty for optional fields.
type ReportExplanation struct {
	ReportType     string             `json:"report_type"`
	Error          string             `json:"error,omitempty"`
	Location       string             `json:"location,omitempty"`
	Timestamp      string             `json:"timestamp,omitempty"`
	IsSpeci        bool               `json:"is_speci,omitempty"`
	IsAutomated    bool               `json:"is_automated,omitempty"`
	IsNil          bool               `json:"is_nil,omitempty"`
	IsCancelled    bool               `json:"is_cancelled,omitempty"`
	IsAmended      bool               `json:"is_amended,omitempty"`
	IsCorrectional bool               `json:"is_correctional,omitempty"`
	Groups         []GroupExplanation `json:"groups"`
}

// ExplainReport parses a METAR/TAF string and returns a structured explanation suitable for serialization.
func ExplainReport(input string) (*ReportExplanation, error) {
	// Ensure input is trimmed and not empty
	trimmedInput := strings.TrimSpace(input)
	if trimmedInput == "" {
		return nil, fmt.Errorf("input report string cannot be empty")
	}

	// Call the SWIG wrapper to parse the report
	// Note: The C++ MetafResult struct is mapped by SWIG to a Go struct.
	// We assume metaf.ParseMetaf returns a struct with accessible fields/methods.
	// Let's assume the returned object `parseResult` has methods like GetReportType(), GetError(), etc.
	// and GetRawGroups() returns an object representing std::vector<std::string> (the explanations).
	parseResult := ParseMetaf(trimmedInput) // Assuming this returns the SWIG proxy object

	// Get the original groups from the input string, handling quoted sections
	originalGroups := splitMaintainingQuotedGroups(trimmedInput)

	// Prepare the structured result
	explanation := &ReportExplanation{
		ReportType:     parseResult.GetReportType(),
		Error:          parseResult.GetError(),
		Location:       parseResult.GetLocation(),
		Timestamp:      parseResult.GetTimestamp(),
		IsSpeci:        parseResult.GetIsSpeci(),
		IsAutomated:    parseResult.GetIsAutomated(),
		IsNil:          parseResult.GetIsNil(),
		IsCancelled:    parseResult.GetIsCancelled(),
		IsAmended:      parseResult.GetIsAmended(),
		IsCorrectional: parseResult.GetIsCorrectional(),
		Groups:         []GroupExplanation{}, // Initialize slice
	}

	// Get the explanations from the parser result
	// Assuming GetRawGroups() returns an object with Size() and Get(i) methods
	// like the original example code structure.
	explanationsVector := parseResult.GetRawGroups()
	numExplanations := int(explanationsVector.Size())
	numRawGroups := len(originalGroups)

	// Determine the number of groups to process (minimum of raw vs explained)
	// This handles potential discrepancies between splitting and parsing.
	numGroupsToProcess := numExplanations
	if numRawGroups < numGroupsToProcess {
		numGroupsToProcess = numRawGroups
	}
	if numExplanations < numRawGroups {
		// This might indicate a parsing issue where some raw groups weren't explained,
		// or a splitting issue. We might want to log this or handle it more robustly.
		// For now, we only process pairs we have explanations for.
		fmt.Printf("Warning: Number of raw groups (%d) differs from number of explanations (%d)\n", numRawGroups, numExplanations)
	}

	// Process each group, pairing raw string with its explanation
	for i := 0; i < numGroupsToProcess; i++ {
		rawGroup := originalGroups[i]
		explanationText := explanationsVector.Get(i) // Get the explanation string

		// Ensure explanationText uses standard newlines \n (no <br>)
		// The C++ part should ideally provide this directly.
		// If the C++ *still* adds <br>, we would need to replace them here:
		// explanationText = strings.ReplaceAll(explanationText, "<br>", "\n")

		explanation.Groups = append(explanation.Groups, GroupExplanation{
			RawGroup:    rawGroup,
			Explanation: explanationText,
		})
	}

	// Handle any remaining raw groups that didn't get an explanation
	if numRawGroups > numExplanations {
		for i := numExplanations; i < numRawGroups; i++ {
			explanation.Groups = append(explanation.Groups, GroupExplanation{
				RawGroup:    originalGroups[i],
				Explanation: "[No explanation generated]", // Indicate missing explanation
			})
		}
	}

	return explanation, nil
}

// splitMaintainingQuotedGroups splits a string by spaces while keeping quoted sections together.
// Copied from original cmd/explain/main.go and adapted slightly.
func splitMaintainingQuotedGroups(s string) []string {
	var result []string
	var current strings.Builder // Use strings.Builder for efficiency
	inQuotes := false

	// Normalize report end marker to space for splitting
	s = strings.Replace(s, "=", " ", -1)
	s = strings.TrimSpace(s) // Trim leading/trailing spaces

	for _, char := range s {
		switch char {
		case '"':
			inQuotes = !inQuotes
			// Decide whether to include quotes in the group or not.
			// The original code skipped them. Let's keep that behaviour.
			// current.WriteRune(char) // Add this line if quotes should be part of the group
		case ' ':
			if !inQuotes {
				if current.Len() > 0 {
					result = append(result, current.String())
					current.Reset()
				}
				// Skip multiple spaces
			} else {
				// Keep spaces inside quotes
				current.WriteRune(char)
			}
		default:
			current.WriteRune(char)
		}
	}

	// Add the last group if any
	if current.Len() > 0 {
		result = append(result, current.String())
	}

	return result
}
