package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strings"

	metaf "github.com/akhenakh/gometaf"
)

// Translation map for common terms
var frenchTranslations = map[string]string{
	// Report types
	"Report Type":   "Type de rapport",
	"METAR":         "METAR",
	"TAF":           "TAF",
	"Parsing Issue": "Problème d'analyse",

	// Metadata
	"Location":     "Emplacement",
	"Timestamp":    "Horodatage",
	"Is SPECI":     "Est SPECI",
	"Is Automated": "Est automatisé",
	"Is NIL":       "Est NIL",
	"Is Cancelled": "Est annulé",
	"Is Amended":   "Est modifié",
	"Is Correctnl": "Est corrigé",
	"true":         "oui",

	// Group titles
	"--- Groups ---": "--- Groupes ---",

	// Report type translations
	"Report type: METAR (weather observation report)": "Type de rapport: METAR (rapport d'observation météorologique)",
	"Report type: TAF (terminal aerodrome forecast)":  "Type de rapport: TAF (prévision d'aérodrome)",
	"Unscheduled METAR (weather observation report)":  "METAR non planifié (rapport d'observation météorologique)",

	// Weather conditions
	"Weather":     "Météo",
	"Temperature": "Température",
	"Visibility":  "Visibilité",
	"Wind":        "Vent",
	"Pressure":    "Pression",
	"Dew point":   "Point de rosée",
	"RH":          "HR", // Relative Humidity - Humidité Relative
	"Cloud layer": "Couche nuageuse",
	"Sky":         "Ciel",

	// Weather phenomena
	"Light":                  "Faible",
	"Moderate":               "Modéré",
	"Heavy":                  "Fort",
	"Vicinity":               "À proximité",
	"Recent":                 "Récent",
	"Shallow":                "Peu profond",
	"Partial":                "Partiel",
	"Patches of":             "Des zones de",
	"Low Drifting":           "Chasse basse",
	"Blowing":                "Soufflant",
	"Showers":                "Averses",
	"Thunderstorm":           "Orage",
	"Freezing":               "Givrant",
	"Drizzle":                "Bruine",
	"Rain":                   "Pluie",
	"Snow":                   "Neige",
	"Fog":                    "Brouillard",
	"Mist":                   "Brume",
	"Haze":                   "Brume sèche",
	"Smoke":                  "Fumée",
	"Volcanic Ash":           "Cendres volcaniques",
	"Dust":                   "Poussière",
	"Sand":                   "Sable",
	"Hail":                   "Grêle",
	"Small Hail":             "Petite grêle",
	"Ice Crystals":           "Cristaux de glace",
	"Ice Pellets":            "Granules de glace",
	"Funnel Cloud":           "Nuage en entonnoir",
	"Dust Storm":             "Tempête de poussière",
	"Sand Storm":             "Tempête de sable",
	"No significant weather": "Pas de météo significative",

	// Sky conditions
	"Clear":                 "Dégagé",
	"No significant clouds": "Pas de nuages significatifs",
	"No clouds detected":    "Aucun nuage détecté",
	"No clouds":             "Aucun nuage",
	"Few clouds":            "Quelques nuages",
	"Scattered clouds":      "Nuages épars",
	"Broken clouds":         "Nuages fragmentés",
	"Overcast":              "Couvert",
	"coverage":              "couverture",
	"at":                    "à",
	"Cumulonimbus":          "Cumulonimbus",
	"Towering Cumulus":      "Cumulus bourgeonnant",
	"Obscuration":           "Obscurcissement",
	"covering":              "couvrant",
	"of the sky":            "du ciel",
	"Vertical visibility":   "Visibilité verticale",
	"not reported":          "non signalé",

	// Wind information
	"from":               "de",
	"degrees":            "degrés",
	"variable direction": "direction variable",
	"gusting to":         "rafales jusqu'à",
	"Calm":               "Calme",
	"knots":              "nœuds",
	"m/s":                "m/s",
	"km/h":               "km/h",
	"mph":                "mph",

	// Visibility
	"meters":              "mètres",
	"statute miles":       "miles terrestres",
	"feet":                "pieds",
	"less than":           "moins de",
	"more than":           "plus de",
	"Runway visual range": "Portée visuelle de piste",
	"for runway":          "pour la piste",
	"Left":                "Gauche",
	"Right":               "Droite",
	"Center":              "Centre",
	"is variable from":    "est variable de",
	"to":                  "à",
	"is":                  "est",
	"increasing":          "en augmentation",
	"decreasing":          "en diminution",
	"no change":           "sans changement",
	"trend":               "tendance",
	"with":                "avec",

	// Pressure
	"hPa":  "hPa",
	"inHg": "inHg",
	"mmHg": "mmHg",

	// Other terms
	"ICAO airport code": "Code d'aéroport OACI",
	"Report time":       "Heure du rapport",
	"day":               "jour",
	"UTC":               "UTC",
	"Fully automated report with no human intervention":     "Rapport entièrement automatisé sans intervention humaine",
	"Ceiling and visibility OK":                             "Plafond et visibilité OK",
	"The remarks are as follows":                            "Les remarques sont les suivantes",
	"Automated station without precipitation discriminator": "Station automatisée sans détecteur de précipitations",
	"Automated station with precipitation discriminator":    "Station automatisée avec détecteur de précipitations",
	"Keyword group":             "Groupe de mots-clés",
	"Additional information":    "Informations supplémentaires",
	"Unknown group":             "Groupe inconnu",
	"Trend information":         "Informations de tendance",
	"Runway state":              "État de la piste",
	"Sea surface conditions":    "Conditions de surface de la mer",
	"Min/Max temperature":       "Température Min/Max",
	"Precipitation information": "Informations sur les précipitations",
	"Layer forecast":            "Prévision de couche",
	"Pressure tendency":         "Tendance de pression",
	"Low/Mid/High clouds":       "Nuages bas/moyens/hauts",
	"Lightning":                 "Foudre",
	"Vicinity observations":     "Observations de proximité",
}

// translateText translates a string from English to French using the translation map
func translateText(text string) string {
	// First try direct translation of the whole string
	if translation, exists := frenchTranslations[text]; exists {
		return translation
	}

	// If no direct translation, try to translate parts
	result := text
	for eng, fr := range frenchTranslations {
		// Replace only complete words or at word boundaries to avoid partial replacements
		result = strings.ReplaceAll(result, " "+eng+" ", " "+fr+" ")
		// Handle beginning of string
		if strings.HasPrefix(result, eng+" ") {
			result = fr + " " + result[len(eng)+1:]
		}
		// Handle end of string
		if strings.HasSuffix(result, " "+eng) {
			result = result[:len(result)-len(eng)-1] + " " + fr
		}
		// Handle standalone words (exact matches)
		if result == eng {
			result = fr
		}
	}

	return result
}

// FrenchExplanation represents the translated explanation structure for JSON output
type FrenchExplanation struct {
	TypeDeRapport   string                `json:"type_de_rapport"`
	ProblemeAnalyse string                `json:"probleme_analyse,omitempty"`
	Emplacement     string                `json:"emplacement,omitempty"`
	Horodatage      string                `json:"horodatage,omitempty"`
	EstSPECI        bool                  `json:"est_speci,omitempty"`
	EstAutomatise   bool                  `json:"est_automatise,omitempty"`
	EstNIL          bool                  `json:"est_nil,omitempty"`
	EstAnnule       bool                  `json:"est_annule,omitempty"`
	EstModifie      bool                  `json:"est_modifie,omitempty"`
	EstCorrige      bool                  `json:"est_corrige,omitempty"`
	Groupes         []GroupeExplicationFr `json:"groupes"`
	RapportOriginal string                `json:"rapport_original,omitempty"`
}

// GroupeExplicationFr represents a translated group explanation for JSON output
type GroupeExplicationFr struct {
	GroupeBrut  string `json:"groupe_brut"`
	Explication string `json:"explication"`
}

func main() {
	reportInput := flag.String("report", "", "Rapport METAR/TAF à analyser")
	formatOutput := flag.String("format", "text", "Format de sortie: 'text' ou 'json'")
	flag.Parse()

	if *reportInput == "" {
		fmt.Fprintln(os.Stderr, "Erreur: Veuillez fournir un rapport METAR/TAF en utilisant l'option -report.")
		flag.Usage()
		os.Exit(1)
	}

	// Check if format is valid
	if *formatOutput != "text" && *formatOutput != "json" {
		fmt.Fprintf(os.Stderr, "Erreur: Format de sortie invalide. Utilisez 'text' ou 'json'.\n")
		os.Exit(1)
	}

	explanation, err := metaf.ExplainReport(*reportInput)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Erreur d'explication du rapport: %v\n", err)
		os.Exit(1)
	}

	// Choose output format
	if *formatOutput == "json" {
		outputJSON(explanation, *reportInput)
	} else {
		outputText(explanation)
	}
}

// outputJSON converts the explanation to French and outputs as JSON
func outputJSON(explanation *metaf.ReportExplanation, originalReport string) {
	// Create French version of the explanation
	frExplanation := FrenchExplanation{
		TypeDeRapport:   explanation.ReportType,
		ProblemeAnalyse: explanation.Error,
		Emplacement:     explanation.Location,
		Horodatage:      translateText(explanation.Timestamp),
		EstSPECI:        explanation.IsSpeci,
		EstAutomatise:   explanation.IsAutomated,
		EstNIL:          explanation.IsNil,
		EstAnnule:       explanation.IsCancelled,
		EstModifie:      explanation.IsAmended,
		EstCorrige:      explanation.IsCorrectional,
		Groupes:         []GroupeExplicationFr{},
		RapportOriginal: originalReport,
	}

	// Translate each group explanation
	for _, group := range explanation.Groups {
		frGroup := GroupeExplicationFr{
			GroupeBrut:  group.RawGroup,
			Explication: translateText(group.Explanation),
		}
		frExplanation.Groupes = append(frExplanation.Groupes, frGroup)
	}

	// Marshal to JSON with indentation for readability
	jsonData, err := json.MarshalIndent(frExplanation, "", "  ")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Erreur lors de la création du JSON: %v\n", err)
		os.Exit(1)
	}

	// Output the JSON
	fmt.Println(string(jsonData))
}

// outputText outputs the explanation in human-readable text format
func outputText(explanation *metaf.ReportExplanation) {
	fmt.Printf("%s: %s\n", translateText("Report Type"), explanation.ReportType)
	if explanation.Error != "" {
		fmt.Printf("%s: %s\n", translateText("Parsing Issue"), explanation.Error)
	}

	// Print metadata flags if they are set or relevant
	if explanation.Location != "" {
		fmt.Printf("%s:    %s\n", translateText("Location"), explanation.Location)
	}
	if explanation.Timestamp != "" {
		fmt.Printf("%s:   %s\n", translateText("Timestamp"), translateText(explanation.Timestamp))
	}
	if explanation.IsSpeci {
		fmt.Printf("%s:    %s\n", translateText("Is SPECI"), translateText("true"))
	}
	if explanation.IsAutomated {
		fmt.Printf("%s:%s\n", translateText("Is Automated"), translateText("true"))
	}
	if explanation.IsNil {
		fmt.Printf("%s:      %s\n", translateText("Is NIL"), translateText("true"))
	}
	if explanation.IsCancelled {
		fmt.Printf("%s:%s\n", translateText("Is Cancelled"), translateText("true"))
	}
	if explanation.IsAmended {
		fmt.Printf("%s:  %s\n", translateText("Is Amended"), translateText("true"))
	}
	if explanation.IsCorrectional {
		fmt.Printf("%s:%s\n", translateText("Is Correctnl"), translateText("true"))
	}

	fmt.Println("\n" + translateText("--- Groups ---"))

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
	alignFormat := fmt.Sprintf("%%-%ds  ", maxGroupLen)

	for _, group := range explanation.Groups {
		// Translate the explanation to French
		translatedExplanation := translateText(group.Explanation)

		// Handle multi-line explanations for aligned plain text output
		lines := strings.Split(translatedExplanation, "\n")
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
