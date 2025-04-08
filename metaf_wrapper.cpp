// metaf_wrapper.cpp
#include "metaf_wrapper.hpp"
#include "./include/metaf.hpp"
#include <sstream>
#include <string>
#include <vector>

// Visitor class to extract information from groups
class SimpleVisitor : public metaf::Visitor<std::string> {
private:
    std::string visitKeywordGroup(const metaf::KeywordGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        switch (group.type()) {
            case metaf::KeywordGroup::Type::METAR:
                return "Report type: METAR (weather observation report)";
            case metaf::KeywordGroup::Type::SPECI:
                return "Unscheduled METAR (weather observation report)";
            case metaf::KeywordGroup::Type::TAF:
                return "Report type: TAF (terminal aerodrome forecast)";
            case metaf::KeywordGroup::Type::AUTO:
                return "Fully automated report with no human intervention";
            case metaf::KeywordGroup::Type::CAVOK:
                return "Ceiling and visibility OK (visibility >10km, no clouds below 5000ft)";
            case metaf::KeywordGroup::Type::RMK:
                return "The remarks are as follows";
            case metaf::KeywordGroup::Type::AO1:
                return "Automated station without precipitation discriminator";
            case metaf::KeywordGroup::Type::AO2:
                return "Automated station with precipitation discriminator";
            default:
                return "Keyword group: " + rawString;
        }
    }

    std::string visitLocationGroup(const metaf::LocationGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "ICAO airport code: " + group.toString();
    }

    std::string visitReportTimeGroup(const metaf::ReportTimeGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;
        const auto& time = group.time();

        result << "Report time: ";
        if (auto day = time.day(); day.has_value())
            result << "day " << *day << ", ";

        result << (time.hour() < 10 ? "0" : "") << time.hour() << ":"
               << (time.minute() < 10 ? "0" : "") << time.minute() << " UTC";

        return result.str();
    }

    std::string visitWindGroup(const metaf::WindGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        if (group.type() == metaf::WindGroup::Type::SURFACE_WIND_CALM) {
            return "Wind: Calm";
        }

        result << "Wind: ";

        if (group.direction().isValue()) {
            result << "from " << *group.direction().degrees() << " degrees";
        } else if (group.direction().type() == metaf::Direction::Type::VARIABLE) {
            result << "variable direction";
        }

        if (group.windSpeed().isReported()) {
            result << " at ";
            if (const auto s = group.windSpeed().speed(); s.has_value()) {
                result << *s << " ";
                switch (group.windSpeed().unit()) {
                    case metaf::Speed::Unit::KNOTS: result << "knots"; break;
                    case metaf::Speed::Unit::METERS_PER_SECOND: result << "m/s"; break;
                    case metaf::Speed::Unit::KILOMETERS_PER_HOUR: result << "km/h"; break;
                    case metaf::Speed::Unit::MILES_PER_HOUR: result << "mph"; break;
                }
            }
        }

        if (group.gustSpeed().isReported()) {
            result << ", gusting to ";
            if (const auto g = group.gustSpeed().speed(); g.has_value()) {
                result << *g << " ";
                switch (group.gustSpeed().unit()) {
                    case metaf::Speed::Unit::KNOTS: result << "knots"; break;
                    case metaf::Speed::Unit::METERS_PER_SECOND: result << "m/s"; break;
                    case metaf::Speed::Unit::KILOMETERS_PER_HOUR: result << "km/h"; break;
                    case metaf::Speed::Unit::MILES_PER_HOUR: result << "mph"; break;
                }
            }
        }

        return result.str();
    }

    std::string visitVisibilityGroup(const metaf::VisibilityGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        switch (group.type()) {
            case metaf::VisibilityGroup::Type::RVR:
            case metaf::VisibilityGroup::Type::VARIABLE_RVR:
                result << "Runway visual range ";
                if (group.runway().has_value()) {
                    result << "for runway " << group.runway()->number();
                    switch (group.runway()->designator()) {
                        case metaf::Runway::Designator::LEFT:
                            result << " Left";
                            break;
                        case metaf::Runway::Designator::RIGHT:
                            result << " Right";
                            break;
                        case metaf::Runway::Designator::CENTER:
                            result << " Center";
                            break;
                        default:
                            break;
                    }
                    result << " ";
                }

                if (group.type() == metaf::VisibilityGroup::Type::VARIABLE_RVR) {
                    result << "is variable from ";
                    if (group.minVisibility().isReported()) {
                        if (group.minVisibility().modifier() == metaf::Distance::Modifier::LESS_THAN) {
                            result << "less than ";
                        } else if (group.minVisibility().modifier() == metaf::Distance::Modifier::MORE_THAN) {
                            result << "more than ";
                        }

                        if (const auto dist = group.minVisibility().distance(); dist.has_value()) {
                            result << *dist << " ";
                            switch (group.minVisibility().unit()) {
                                case metaf::Distance::Unit::METERS:
                                    result << "meters";
                                    break;
                                case metaf::Distance::Unit::STATUTE_MILES:
                                    result << "statute miles";
                                    break;
                                case metaf::Distance::Unit::FEET:
                                    result << "feet";
                                    break;
                            }
                        }
                    }

                    result << " to ";

                    if (group.maxVisibility().isReported()) {
                        if (group.maxVisibility().modifier() == metaf::Distance::Modifier::LESS_THAN) {
                            result << "less than ";
                        } else if (group.maxVisibility().modifier() == metaf::Distance::Modifier::MORE_THAN) {
                            result << "more than ";
                        }

                        if (const auto dist = group.maxVisibility().distance(); dist.has_value()) {
                            result << *dist << " ";
                            switch (group.maxVisibility().unit()) {
                                case metaf::Distance::Unit::METERS:
                                    result << "meters";
                                    break;
                                case metaf::Distance::Unit::STATUTE_MILES:
                                    result << "statute miles";
                                    break;
                                case metaf::Distance::Unit::FEET:
                                    result << "feet";
                                    break;
                            }
                        }
                    }
                } else {
                    result << "is ";

                    if (!group.visibility().isReported()) {
                        return result.str() + "not reported";
                    }

                    if (group.visibility().modifier() == metaf::Distance::Modifier::LESS_THAN) {
                        result << "less than ";
                    } else if (group.visibility().modifier() == metaf::Distance::Modifier::MORE_THAN) {
                        result << "more than ";
                    }

                    if (const auto dist = group.visibility().distance(); dist.has_value()) {
                        result << *dist << " ";
                        switch (group.visibility().unit()) {
                            case metaf::Distance::Unit::METERS:
                                result << "meters";
                                break;
                            case metaf::Distance::Unit::STATUTE_MILES:
                                result << "statute miles";
                                break;
                            case metaf::Distance::Unit::FEET:
                                result << "feet";
                                break;
                        }
                    }
                }

                // Add conversion information for better understanding
                result << " (";

                if (group.type() == metaf::VisibilityGroup::Type::VARIABLE_RVR) {
                    // For minimum visibility
                    if (const auto minMeterDist = group.minVisibility().toUnit(metaf::Distance::Unit::METERS);
                        minMeterDist.has_value() && group.minVisibility().unit() != metaf::Distance::Unit::METERS) {
                        result << static_cast<int>(*minMeterDist) << " meters";
                    }
                    if (const auto minMilesDist = group.minVisibility().toUnit(metaf::Distance::Unit::STATUTE_MILES);
                        minMilesDist.has_value() && group.minVisibility().unit() != metaf::Distance::Unit::STATUTE_MILES) {
                        if (group.minVisibility().unit() != metaf::Distance::Unit::METERS) {
                            result << " / ";
                        }
                        result << *minMilesDist << " statute miles";
                    }

                    result << " to ";

                    // For maximum visibility
                    if (const auto maxMeterDist = group.maxVisibility().toUnit(metaf::Distance::Unit::METERS);
                        maxMeterDist.has_value() && group.maxVisibility().unit() != metaf::Distance::Unit::METERS) {
                        result << static_cast<int>(*maxMeterDist) << " meters";
                    }
                    if (const auto maxMilesDist = group.maxVisibility().toUnit(metaf::Distance::Unit::STATUTE_MILES);
                        maxMilesDist.has_value() && group.maxVisibility().unit() != metaf::Distance::Unit::STATUTE_MILES) {
                        if (group.maxVisibility().unit() != metaf::Distance::Unit::METERS) {
                            result << " / ";
                        }
                        result << *maxMilesDist << " statute miles";
                    }
                } else {
                    // For regular visibility
                    if (const auto meterDist = group.visibility().toUnit(metaf::Distance::Unit::METERS);
                        meterDist.has_value() && group.visibility().unit() != metaf::Distance::Unit::METERS) {
                        result << static_cast<int>(*meterDist) << " meters";
                    }
                    if (const auto milesDist = group.visibility().toUnit(metaf::Distance::Unit::STATUTE_MILES);
                        milesDist.has_value() && group.visibility().unit() != metaf::Distance::Unit::STATUTE_MILES) {
                        if (group.visibility().unit() != metaf::Distance::Unit::METERS) {
                            result << " / ";
                        }
                        result << *milesDist << " statute miles";
                    }
                }

                result << ")";

                if (group.trend() != metaf::VisibilityGroup::Trend::NONE &&
                    group.trend() != metaf::VisibilityGroup::Trend::NOT_REPORTED) {
                    result << ", with ";
                    switch (group.trend()) {
                        case metaf::VisibilityGroup::Trend::UPWARD:
                            result << "increasing";
                            break;
                        case metaf::VisibilityGroup::Trend::DOWNWARD:
                            result << "decreasing";
                            break;
                        case metaf::VisibilityGroup::Trend::NEUTRAL:
                            result << "no change";
                            break;
                        default:
                            break;
                    }
                    result << " trend";
                }

                return result.str();

            case metaf::VisibilityGroup::Type::TOWER:
                result << "Visibility from air traffic control tower is ";
                break;
            case metaf::VisibilityGroup::Type::SURFACE:
                result << "Surface visibility is ";
                break;
            case metaf::VisibilityGroup::Type::RUNWAY:
                result << "Runway visibility is ";
                break;
            case metaf::VisibilityGroup::Type::PREVAILING:
            default:
                result << "Visibility: ";
                break;
        }

        if (!group.visibility().isReported()) {
            return result.str() + "not reported";
        }

        if (group.visibility().modifier() == metaf::Distance::Modifier::LESS_THAN) {
            result << "less than ";
        } else if (group.visibility().modifier() == metaf::Distance::Modifier::MORE_THAN) {
            result << "more than ";
        }

        if (const auto dist = group.visibility().distance(); dist.has_value()) {
            result << *dist << " ";
            switch (group.visibility().unit()) {
                case metaf::Distance::Unit::METERS:
                    result << "meters";
                    break;
                case metaf::Distance::Unit::STATUTE_MILES:
                    result << "statute miles";
                    break;
                case metaf::Distance::Unit::FEET:
                    result << "feet";
                    break;
            }

            // Add conversion for better understanding
            if (group.visibility().unit() == metaf::Distance::Unit::STATUTE_MILES) {
                if (const auto meters = group.visibility().toUnit(metaf::Distance::Unit::METERS); meters.has_value()) {
                    result << " (" << static_cast<int>(*meters) << " meters";

                    if (const auto feet = group.visibility().toUnit(metaf::Distance::Unit::FEET); feet.has_value()) {
                        result << " / " << static_cast<int>(*feet) << " feet";
                    }

                    result << ")";
                }
            }
        }

        return result.str();
    }

    std::string visitCloudGroup(const metaf::CloudGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        switch (group.type()) {
            case metaf::CloudGroup::Type::NO_CLOUDS:
                switch (group.amount()) {
                    case metaf::CloudGroup::Amount::NONE_CLR:
                    case metaf::CloudGroup::Amount::NONE_SKC:
                        return "Sky: Clear";
                    case metaf::CloudGroup::Amount::NSC:
                        return "Sky: No significant clouds";
                    case metaf::CloudGroup::Amount::NCD:
                        return "Sky: No clouds detected";
                    default:
                        return "Sky: No clouds";
                }

            case metaf::CloudGroup::Type::CLOUD_LAYER:
                result << "Cloud layer: ";
                switch (group.amount()) {
                    case metaf::CloudGroup::Amount::FEW:
                        result << "Few clouds (1/8 to 2/8 coverage)";
                        break;
                    case metaf::CloudGroup::Amount::SCATTERED:
                        result << "Scattered clouds (3/8 to 4/8 coverage)";
                        break;
                    case metaf::CloudGroup::Amount::BROKEN:
                        result << "Broken clouds (5/8 to 7/8 coverage)";
                        break;
                    case metaf::CloudGroup::Amount::OVERCAST:
                        result << "Overcast (8/8 coverage)";
                        break;
                    default:
                        result << "Unknown amount";
                        break;
                }

                if (const auto h = group.height().distance(); h.has_value()) {
                    result << " at " << *h << " feet";
                }

                if (group.convectiveType() == metaf::CloudGroup::ConvectiveType::CUMULONIMBUS) {
                    result << " (Cumulonimbus)";
                } else if (group.convectiveType() == metaf::CloudGroup::ConvectiveType::TOWERING_CUMULUS) {
                    result << " (Towering Cumulus)";
                }
                break;

            case metaf::CloudGroup::Type::VERTICAL_VISIBILITY:
                result << "Vertical visibility: ";
                if (const auto v = group.verticalVisibility().distance(); v.has_value()) {
                    result << *v  << " feet";
                } else {
                    result << "not reported";
                }
                break;

            case metaf::CloudGroup::Type::OBSCURATION:
                if (const auto ct = group.cloudType(); ct.has_value()) {
                    result << "Obscuration: ";
                    // Handle cloud type
                    switch (ct->type()) {
                        case metaf::CloudType::Type::SNOW:
                            result << "snow";
                            break;
                        case metaf::CloudType::Type::FOG:
                            result << "fog";
                            break;
                        case metaf::CloudType::Type::SMOKE:
                            result << "smoke";
                            break;
                        case metaf::CloudType::Type::VOLCANIC_ASH:
                            result << "volcanic ash";
                            break;
                        case metaf::CloudType::Type::HAZE:
                            result << "haze";
                            break;
                        case metaf::CloudType::Type::MIST:
                            result << "mist";
                            break;
                        default:
                            result << "unknown";
                            break;
                    }
                    result << " covering " << ct->okta() << "/8 of the sky";
                } else {
                    result << "Obscuration";
                }
                break;

            default:
                result << "Cloud information: " << rawString;
                break;
        }

        return result.str();
    }

    std::string visitTemperatureGroup(const metaf::TemperatureGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        result << "Temperature: ";
        if (const auto t = group.airTemperature().temperature(); t.has_value()) {
            result << *t << "°C";

            if (const auto tF = group.airTemperature().toUnit(metaf::Temperature::Unit::F); tF.has_value()) {
                result << " (" << static_cast<int>(*tF) << "°F)";
            }
        } else {
            result << "not reported";
        }

        result << ", Dew point: ";
        if (const auto dp = group.dewPoint().temperature(); dp.has_value()) {
            result << *dp << "°C";

            if (const auto dpF = group.dewPoint().toUnit(metaf::Temperature::Unit::F); dpF.has_value()) {
                result << " (" << static_cast<int>(*dpF) << "°F)";
            }
        } else {
            result << "not reported";
        }

        if (const auto rh = group.relativeHumidity(); rh.has_value()) {
            result << " (RH: " << static_cast<int>(*rh) << "%)";
        }

        return result.str();
    }

    std::string visitPressureGroup(const metaf::PressureGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        result << "Pressure: ";
        if (const auto p = group.atmosphericPressure().pressure(); p.has_value()) {
            switch (group.atmosphericPressure().unit()) {
                case metaf::Pressure::Unit::HECTOPASCAL:
                    result << *p << " hPa";
                    break;
                case metaf::Pressure::Unit::INCHES_HG:
                    result << *p << " inHg";

                    if (const auto phPa = group.atmosphericPressure().toUnit(metaf::Pressure::Unit::HECTOPASCAL); phPa.has_value()) {
                        result << " (" << static_cast<int>(*phPa) << " hPa)";
                    }
                    break;
                case metaf::Pressure::Unit::MM_HG:
                    result << *p << " mmHg";
                    break;
            }
        } else {
            result << "not reported";
        }

        return result.str();
    }

    std::string visitWeatherGroup(const metaf::WeatherGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;

        if (group.type() == metaf::WeatherGroup::Type::NSW) {
            return "Weather: No significant weather";
        }

        result << "Weather: ";

        for (const auto& wp : group.weatherPhenomena()) {
            // Weather qualifier
            switch (wp.qualifier()) {
                case metaf::WeatherPhenomena::Qualifier::LIGHT:
                    result << "Light ";
                    break;
                case metaf::WeatherPhenomena::Qualifier::MODERATE:
                    result << "Moderate ";
                    break;
                case metaf::WeatherPhenomena::Qualifier::HEAVY:
                    result << "Heavy ";
                    break;
                case metaf::WeatherPhenomena::Qualifier::VICINITY:
                    result << "Vicinity ";
                    break;
                case metaf::WeatherPhenomena::Qualifier::RECENT:
                    result << "Recent ";
                    break;
                default:
                    break;
            }

            // Weather descriptor
            switch (wp.descriptor()) {
                case metaf::WeatherPhenomena::Descriptor::SHALLOW:
                    result << "Shallow ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::PARTIAL:
                    result << "Partial ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::PATCHES:
                    result << "Patches of ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING:
                    result << "Low Drifting ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::BLOWING:
                    result << "Blowing ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::SHOWERS:
                    result << "Showers ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::THUNDERSTORM:
                    result << "Thunderstorm ";
                    break;
                case metaf::WeatherPhenomena::Descriptor::FREEZING:
                    result << "Freezing ";
                    break;
                default:
                    break;
            }

            // Weather phenomena
            for (const auto& w : wp.weather()) {
                switch (w) {
                    case metaf::WeatherPhenomena::Weather::DRIZZLE:
                        result << "Drizzle ";
                        break;
                    case metaf::WeatherPhenomena::Weather::RAIN:
                        result << "Rain ";
                        break;
                    case metaf::WeatherPhenomena::Weather::SNOW:
                        result << "Snow ";
                        break;
                    case metaf::WeatherPhenomena::Weather::FOG:
                        result << "Fog ";
                        break;
                    case metaf::WeatherPhenomena::Weather::MIST:
                        result << "Mist ";
                        break;
                    case metaf::WeatherPhenomena::Weather::HAZE:
                        result << "Haze ";
                        break;
                    case metaf::WeatherPhenomena::Weather::SMOKE:
                        result << "Smoke ";
                        break;
                    case metaf::WeatherPhenomena::Weather::VOLCANIC_ASH:
                        result << "Volcanic Ash ";
                        break;
                    case metaf::WeatherPhenomena::Weather::DUST:
                        result << "Dust ";
                        break;
                    case metaf::WeatherPhenomena::Weather::SAND:
                        result << "Sand ";
                        break;
                    case metaf::WeatherPhenomena::Weather::HAIL:
                        result << "Hail ";
                        break;
                    case metaf::WeatherPhenomena::Weather::SMALL_HAIL:
                        result << "Small Hail ";
                        break;
                    case metaf::WeatherPhenomena::Weather::ICE_CRYSTALS:
                        result << "Ice Crystals ";
                        break;
                    case metaf::WeatherPhenomena::Weather::ICE_PELLETS:
                        result << "Ice Pellets ";
                        break;
                    case metaf::WeatherPhenomena::Weather::FUNNEL_CLOUD:
                        result << "Funnel Cloud ";
                        break;
                    case metaf::WeatherPhenomena::Weather::DUSTSTORM:
                        result << "Dust Storm ";
                        break;
                    case metaf::WeatherPhenomena::Weather::SANDSTORM:
                        result << "Sand Storm ";
                        break;
                    default:
                        break;
                }
            }
        }

        return result.str();
    }

    std::string visitCloudTypesGroup(const metaf::CloudTypesGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        std::ostringstream result;
        result << "Obscuration / cloud layers:";

        for (const auto& ct : group.cloudTypes()) {
            result << "\n";

            switch (ct.type()) {
                case metaf::CloudType::Type::SNOW:
                    result << "snow";
                    break;
                case metaf::CloudType::Type::FOG:
                    result << "fog";
                    break;
                case metaf::CloudType::Type::SMOKE:
                    result << "smoke";
                    break;
                case metaf::CloudType::Type::VOLCANIC_ASH:
                    result << "volcanic ash";
                    break;
                case metaf::CloudType::Type::HAZE:
                    result << "haze";
                    break;
                case metaf::CloudType::Type::MIST:
                    result << "mist";
                    break;
                case metaf::CloudType::Type::CUMULONIMBUS:
                    result << "cumulonimbus";
                    break;
                case metaf::CloudType::Type::TOWERING_CUMULUS:
                    result << "towering cumulus";
                    break;
                case metaf::CloudType::Type::CUMULUS:
                    result << "cumulus";
                    break;
                case metaf::CloudType::Type::CUMULUS_FRACTUS:
                    result << "cumulus fractus";
                    break;
                case metaf::CloudType::Type::STRATOCUMULUS:
                    result << "stratocumulus";
                    break;
                case metaf::CloudType::Type::NIMBOSTRATUS:
                    result << "nimbostratus";
                    break;
                case metaf::CloudType::Type::STRATUS:
                    result << "stratus";
                    break;
                case metaf::CloudType::Type::STRATUS_FRACTUS:
                    result << "stratus fractus";
                    break;
                case metaf::CloudType::Type::ALTOSTRATUS:
                    result << "altostratus";
                    break;
                case metaf::CloudType::Type::ALTOCUMULUS:
                    result << "altocumulus";
                    break;
                case metaf::CloudType::Type::CIRRUS:
                    result << "cirrus";
                    break;
                case metaf::CloudType::Type::CIRROSTRATUS:
                    result << "cirrostratus";
                    break;
                case metaf::CloudType::Type::CIRROCUMULUS:
                    result << "cirrocumulus";
                    break;
                default:
                    result << "unknown";
                    break;
            }

            result << " covering " << ct.okta() << "/8 of the sky";

            if (const auto h = ct.height().distance(); h.has_value()) {
                result << " at " << *h  << " feet";
            }
        }

        return result.str();
    }

    std::string visitMiscGroup(const metaf::MiscGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Additional information: " + rawString;
    }

    std::string visitUnknownGroup(const metaf::UnknownGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Unknown group: " + rawString;
    }

    // Default implementations for other group types
    std::string visitTrendGroup(const metaf::TrendGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Trend information: " + rawString;
    }

    std::string visitRunwayStateGroup(const metaf::RunwayStateGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Runway state: " + rawString;
    }

    std::string visitSeaSurfaceGroup(const metaf::SeaSurfaceGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Sea surface conditions: " + rawString;
    }

    std::string visitMinMaxTemperatureGroup(const metaf::MinMaxTemperatureGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Min/Max temperature: " + rawString;
    }

    std::string visitPrecipitationGroup(const metaf::PrecipitationGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Precipitation information: " + rawString;
    }

    std::string visitLayerForecastGroup(const metaf::LayerForecastGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Layer forecast: " + rawString;
    }

    std::string visitPressureTendencyGroup(const metaf::PressureTendencyGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Pressure tendency: " + rawString;
    }

    std::string visitLowMidHighCloudGroup(const metaf::LowMidHighCloudGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Low/Mid/High clouds: " + rawString;
    }

    std::string visitLightningGroup(const metaf::LightningGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Lightning: " + rawString;
    }

    std::string visitVicinityGroup(const metaf::VicinityGroup & group, metaf::ReportPart reportPart, const std::string & rawString) override {
        return "Vicinity observations: " + rawString;
    }
};

// Parse a METAR/TAF report and return a structured result
MetafResult ParseMetaf(const std::string& report) {
    MetafResult result;

    // Parse the report using metaf library
    auto parseResult = metaf::Parser::parse(report);

    // Extract metadata
    switch (parseResult.reportMetadata.type) {
        case metaf::ReportType::METAR:
            result.reportType = "METAR";
            break;
        case metaf::ReportType::TAF:
            result.reportType = "TAF";
            break;
        default:
            result.reportType = "UNKNOWN";
            break;
    }

    // Set error if any
    switch (parseResult.reportMetadata.error) {
        case metaf::ReportError::NONE:
            result.error = "";
            break;
        case metaf::ReportError::EMPTY_REPORT:
            result.error = "Empty report";
            break;
        case metaf::ReportError::EXPECTED_REPORT_TYPE_OR_LOCATION:
            result.error = "Expected report type or location";
            break;
        case metaf::ReportError::EXPECTED_LOCATION:
            result.error = "Expected location";
            break;
        case metaf::ReportError::EXPECTED_REPORT_TIME:
            result.error = "Expected report time";
            break;
        case metaf::ReportError::EXPECTED_TIME_SPAN:
            result.error = "Expected time span";
            break;
        case metaf::ReportError::UNEXPECTED_REPORT_END:
            result.error = "Unexpected report end";
            break;
        default:
            result.error = "Parsing error";
            break;
    }

    // Set other metadata
    result.location = parseResult.reportMetadata.icaoLocation;
    if (parseResult.reportMetadata.reportTime.has_value()) {
        std::ostringstream timeStr;
        const auto& time = *parseResult.reportMetadata.reportTime;
        if (auto day = time.day(); day.has_value())
            timeStr << "day " << *day << ", ";
        timeStr << (time.hour() < 10 ? "0" : "") << time.hour() << ":"
                << (time.minute() < 10 ? "0" : "") << time.minute() << " UTC";
        result.timestamp = timeStr.str();
    }

    result.isSpeci = parseResult.reportMetadata.isSpeci;
    result.isAutomated = parseResult.reportMetadata.isAutomated;
    result.isNil = parseResult.reportMetadata.isNil;
    result.isCancelled = parseResult.reportMetadata.isCancelled;
    result.isAmended = parseResult.reportMetadata.isAmended;
    result.isCorrectional = parseResult.reportMetadata.isCorrectional;

    // Process each group and explain it
    SimpleVisitor visitor;

    // Process each parsed group
    for (const auto& groupInfo : parseResult.groups) {
        // Get explanation for the group
        std::string explanation = visitor.visit(groupInfo.group, groupInfo.reportPart, groupInfo.rawString);

        // Add raw group and explanation to the result
        result.rawGroups.push_back(explanation);
    }

    return result;
}
