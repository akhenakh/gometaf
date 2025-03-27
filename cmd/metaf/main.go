package main

import (
	"fmt"

	metaf "github.com/akhenakh/gometaf"
)

func main() {
	reportText := "METAR KBOS 011254Z 09007KT 10SM FEW020 FEW100 26/22 A3000 RMK AO2"

	result := metaf.ParseMetaf(reportText)

	fmt.Println("Report Type:", result.GetReportType())
	fmt.Println("Location:", result.GetLocation())
	fmt.Println("Timestamp:", result.GetTimestamp())

	fmt.Println("\nRaw Groups:")
	groups := result.GetRawGroups()
	for i := 0; i < int(groups.Size()); i++ {
		fmt.Printf("  %s\n", groups.Get(i))
	}
}
