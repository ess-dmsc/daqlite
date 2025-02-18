#pragma once
#include <QSpinBox>

/// \brief A specialization of Qt QSpinBox which allows only values which are powers of two
class TwoSpinBox: public QSpinBox
{
public:
    TwoSpinBox(int minimum, int maximum): QSpinBox()
    {
        for (int i=minimum; i<=maximum; i<<=1) {
            acceptedValues << i;
        }
        setRange(acceptedValues.first(), acceptedValues.last());
        setValue(maximum);
    }

    void stepBy(int steps) override
    {
         // Bounds the index between 0 and length
        const int length = static_cast<int>((acceptedValues.indexOf(value()) + steps) % acceptedValues.length());
        const int index = std::max(0, length);
        setValue(acceptedValues.value(index));
    }
private:
    QList<int> acceptedValues;
};