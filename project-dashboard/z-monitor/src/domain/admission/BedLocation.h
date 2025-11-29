/**
 * @file BedLocation.h
 * @brief Value object representing a bed/room location.
 * 
 * This file contains the BedLocation value object which represents a bed/room
 * location identifier (e.g., "ICU-4B", "Room 201-Bed 1"). Value objects are
 * immutable and defined by their attributes.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <string>

namespace zmon {
/**
 * @class BedLocation
 * @brief Immutable value object representing a bed/room location.
 * 
 * This value object encapsulates a bed/room location identifier. It is
 * immutable and can be safely shared across threads.
 * 
 * @note This is a value object - it has no identity and is defined by its attributes.
 * @note All members are const to enforce immutability.
 */
struct BedLocation {
    /**
     * @brief Bed/room location identifier.
     * 
     * Examples: "ICU-4B", "Room 201-Bed 1", "ER-Bay 3".
     */
    const std::string location;
    
    /**
     * @brief Unit/facility identifier (optional).
     * 
     * Examples: "ICU", "ER", "Ward 2".
     */
    const std::string unit;
    
    /**
     * @brief Default constructor.
     * 
     * Creates an empty bed location with default values.
     */
    BedLocation()
        : location("")
        , unit("")
    {}
    
    /**
     * @brief Constructor with location string.
     * 
     * @param loc Bed/room location identifier
     */
    explicit BedLocation(const std::string& loc)
        : location(loc)
        , unit("")
    {}
    
    /**
     * @brief Constructor with location and unit.
     * 
     * @param loc Bed/room location identifier
     * @param unitName Unit/facility identifier
     */
    BedLocation(const std::string& loc, const std::string& unitName)
        : location(loc)
        , unit(unitName)
    {}
    
    /**
     * @brief Copy constructor.
     * 
     * @param other Source bed location
     */
    BedLocation(const BedLocation& other) = default;
    
    /**
     * @brief Assignment operator (deleted - value objects are immutable).
     */
    BedLocation& operator=(const BedLocation&) = delete;
    
    /**
     * @brief Equality comparison.
     * 
     * Two bed locations are equal if their location and unit match.
     * 
     * @param other Other bed location to compare
     * @return true if location and unit are equal, false otherwise
     */
    bool operator==(const BedLocation& other) const {
        return location == other.location && unit == other.unit;
    }
    
    /**
     * @brief Inequality comparison.
     * 
     * @param other Other bed location to compare
     * @return true if location or unit differs, false otherwise
     */
    bool operator!=(const BedLocation& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Check if bed location is valid (has location string).
     * 
     * @return true if location is not empty, false otherwise
     */
    bool isValid() const {
        return !location.empty();
    }
    
    /**
     * @brief Get full location string (unit + location).
     * 
     * @return Full location string (e.g., "ICU-4B" or "ICU / 4B")
     */
    std::string toString() const {
        if (unit.empty()) {
            return location;
        }
        return unit + "-" + location;
    }
};

} // namespace zmon