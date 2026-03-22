#include "converter.h"

// -----------------------------------------------------------------------
// Unit lists — order must match ComboBox items in mainwindow.ui
// -----------------------------------------------------------------------
// Length  : 0=m, 1=km, 2=in, 3=ft, 4=mi
// Mass    : 0=kg, 1=lb, 2=oz
// Temp    : 0=°C, 1=°F, 2=K

QStringList Converter::units(Mode mode)
{
    switch (mode) {
    case Mode::Length:
        return { "meters (m)", "kilometers (km)", "inches (in)", "feet (ft)", "miles (mi)" };
    case Mode::Mass:
        return { "kilograms (kg)", "pounds (lb)", "ounces (oz)" };
    case Mode::Temperature:
        return { "Celsius (°C)", "Fahrenheit (°F)", "Kelvin (K)" };
    }
    return {};
}

// -----------------------------------------------------------------------
// Length — base: meter
// -----------------------------------------------------------------------
double Converter::lengthToBase(double v, int i)
{
    // coefficients: how many meters in 1 unit
    constexpr double toMeter[] = { 1.0, 1000.0, 0.0254, 0.3048, 1609.344 };
    if (i < 0 || i > 4) return 0.0;
    return v * toMeter[i];
}

double Converter::lengthFromBase(double base, int i)
{
    constexpr double toMeter[] = { 1.0, 1000.0, 0.0254, 0.3048, 1609.344 };
    if (i < 0 || i > 4) return 0.0;
    return base / toMeter[i];
}

// -----------------------------------------------------------------------
// Mass — base: kilogram
// -----------------------------------------------------------------------
double Converter::massToBase(double v, int i)
{
    // coefficients: how many kg in 1 unit
    constexpr double toKg[] = { 1.0, 0.45359237, 0.028349523125 };
    if (i < 0 || i > 2) return 0.0;
    return v * toKg[i];
}

double Converter::massFromBase(double base, int i)
{
    constexpr double toKg[] = { 1.0, 0.45359237, 0.028349523125 };
    if (i < 0 || i > 2) return 0.0;
    return base / toKg[i];
}

// -----------------------------------------------------------------------
// Temperature — base: kelvin
// -----------------------------------------------------------------------
double Converter::tempToBase(double v, int i)
{
    switch (i) {
    case 0: return v + 273.15;           // °C → K
    case 1: return (v - 32.0) * 5.0 / 9.0 + 273.15; // °F → K
    case 2: return v;                    // K → K
    default: return 0.0;
    }
}

double Converter::tempFromBase(double base, int i)
{
    switch (i) {
    case 0: return base - 273.15;        // K → °C
    case 1: return (base - 273.15) * 9.0 / 5.0 + 32.0; // K → °F
    case 2: return base;                 // K → K
    default: return 0.0;
    }
}

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------
double Converter::convert(Mode mode, double value, int fromIndex, int toIndex)
{
    switch (mode) {
    case Mode::Length:
        return lengthFromBase(lengthToBase(value, fromIndex), toIndex);
    case Mode::Mass:
        return massFromBase(massToBase(value, fromIndex), toIndex);
    case Mode::Temperature:
        return tempFromBase(tempToBase(value, fromIndex), toIndex);
    }
    return 0.0;
}

QString Converter::format(double value)
{
    return QString::number(value, 'g', 10);
}
