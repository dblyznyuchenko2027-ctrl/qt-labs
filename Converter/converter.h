#pragma once

#include <QString>
#include <QStringList>

// -----------------------------------------------------------------------
// Converter — pure conversion logic, no UI dependencies.
// All conversions go through a base unit (meter / kilogram / kelvin).
// -----------------------------------------------------------------------
class Converter
{
public:
    enum class Mode { Length, Mass, Temperature };

    // Returns the unit list for a given mode (same order as ComboBox items)
    static QStringList units(Mode mode);

    // Convert value from unit[fromIndex] to unit[toIndex] for the given mode.
    // Returns converted value. On error (unsupported index) returns 0.
    static double convert(Mode mode, double value, int fromIndex, int toIndex);

    // Format a result number for display (10 significant figures, 'g' format)
    static QString format(double value);

private:
    // Length helpers — base unit: meter
    static double lengthToBase(double v, int unitIndex);
    static double lengthFromBase(double base, int unitIndex);

    // Mass helpers — base unit: kilogram
    static double massToBase(double v, int unitIndex);
    static double massFromBase(double base, int unitIndex);

    // Temperature helpers — base unit: kelvin
    static double tempToBase(double v, int unitIndex);
    static double tempFromBase(double base, int unitIndex);
};
